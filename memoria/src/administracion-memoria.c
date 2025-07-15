#include "administracion-memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>


bool *bitmap_frames = NULL;
void *memoria_principal ;

t_diccionarioConSemaforos* diccionarioProcesos;

typedef struct {
  uint32_t pid;
  uint32_t offset;
  uint32_t tamanio_original;
} EntradaSwap;

t_list* tabla_swap;


void leerConfigMemoria(t_config* config_memoria) 
{
    puerto_escucha = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");
    TAM_MEMORIA = config_get_int_value(config_memoria, "TAM_MEMORIA");
    TAM_PAGINA = config_get_int_value(config_memoria, "TAM_PAGINA");
    ENTRADAS_POR_TABLA = config_get_int_value(config_memoria, "ENTRADAS_POR_TABLA");
    CANTIDAD_NIVELES = config_get_int_value(config_memoria, "CANTIDAD_NIVELES");
    retardo_memoria = config_get_int_value(config_memoria, "RETARDO_MEMORIA");
    path_swapfile = config_get_string_value(config_memoria, "PATH_SWAPFILE");
    retardo_swap = config_get_int_value(config_memoria, "RETARDO_SWAP");
    log_level = log_level_from_string(config_get_string_value(config_memoria, "LOG_LEVEL"));
    dump_path = config_get_string_value(config_memoria, "DUMP_PATH"); 
    path_pseudocodigos = config_get_string_value(config_memoria,"PATH_PSEUDOCODIGOS");
}


// Inicializa la memoria simulada y el bitmap de frames libres
void inicializar_memoria() {
  
  CANT_FRAMES = TAM_MEMORIA / TAM_PAGINA;

  memoria_principal = malloc(TAM_MEMORIA);
  memset(memoria_principal, 0, TAM_MEMORIA);

  if (!memoria_principal) {
    log_error(logger_memoria, "No se pudo asignar memoria simulada\n");
    exit(EXIT_FAILURE);
  }

  bitmap_frames = malloc(sizeof(bool) * CANT_FRAMES);
  if (!bitmap_frames) {
    log_error(logger_memoria, "No se pudo asignar bitmap de frames\n");
    free(memoria_principal);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < CANT_FRAMES; i++) {
    bitmap_frames[i] = false; // false indica frame libre
  }

  tabla_swap = list_create();
  if (!tabla_swap) {
    log_error(logger_memoria, "No se pudo crear la lista de swap\n");
  }

  FILE *archivo = fopen(path_swapfile, "wb");
  if (!archivo) {
      log_error(logger_memoria, "No se pudo crear swapfile.bin");
  }
  fclose(archivo);

  log_info(logger_memoria, "## Memoria inicializada");
  log_debug(logger_memoria,  "Cantidad de frames: <%d>",  CANT_FRAMES);
  log_debug(logger_memoria,"Tamaño de página: <%d>",  TAM_PAGINA);
}

// Crea tabla multinivel recursivamente
TablaPagina *crear_tabla_nivel(int nivel_actual) {
  TablaPagina *tabla = malloc(sizeof(TablaPagina));
  tabla->es_hoja = (nivel_actual == CANTIDAD_NIVELES);

  if (tabla->es_hoja) {
    tabla->entradas = NULL;
    tabla->frames = malloc(sizeof(int) * ENTRADAS_POR_TABLA);
    for (int i = 0; i < ENTRADAS_POR_TABLA; i++)
      tabla->frames[i] = -1; // sin frame asignado aún
  } else {
    tabla->frames = NULL;
    tabla->entradas = calloc(ENTRADAS_POR_TABLA, sizeof(TablaPagina *));
    }
  
  return tabla;
}

void asignar_frames_en_tabla(TablaPagina *tabla, int paginas_a_reservar, int *frames, int *pagina_logica_actual, int nivel_actual, Proceso *p) {
  if (*pagina_logica_actual >= paginas_a_reservar)
    return;

  p->metricas.accesos_tabla_paginas++;

  for (int i = 0;
       i < ENTRADAS_POR_TABLA && *pagina_logica_actual < paginas_a_reservar;
       i++) {
    if (tabla->es_hoja) {
      if (tabla->frames[i] == -1) {
        tabla->frames[i] = frames[*pagina_logica_actual];
        (*pagina_logica_actual)++;
      }
    } else {
      if (*pagina_logica_actual < paginas_a_reservar) {
      if (tabla->entradas[i] == NULL) {
        tabla->entradas[i] = crear_tabla_nivel(nivel_actual + 1);
      }
      asignar_frames_en_tabla(tabla->entradas[i], paginas_a_reservar, frames, pagina_logica_actual, nivel_actual + 1, p);
      }
    }
  }
}

void imprimir_tabla(TablaPagina *tabla, int nivel_actual, int indent) {
  for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
    if (tabla->es_hoja) {
      int frame = tabla->frames[i];
      if (frame == -1) {
        for (int j = 0; j < indent; j++) log_debug(logger_memoria,"  ");
        log_debug(logger_memoria,"Nivel %d - Entrada %d: [SIN FRAME]\n", nivel_actual, i);

      } else {
        for (int j = 0; j < indent; j++) log_debug(logger_memoria,"  ");
        log_debug(logger_memoria,"Nivel %d - Entrada %d: Frame %d\n", nivel_actual, i, frame);
      }
    } else {

      if (tabla->entradas == NULL) {
        log_debug(logger_memoria,"  (entradas == NULL)\n");
        return;
    }
      if(tabla->entradas[i] != NULL){
        for (int j = 0; j < indent; j++) log_debug(logger_memoria,"  ");
        log_debug(logger_memoria,"Nivel %d - Entrada %d:\n", nivel_actual, i);
        imprimir_tabla(tabla->entradas[i], nivel_actual + 1, indent + 1);
      }

    }
  }
}

void liberar_tabla(TablaPagina *tabla) {
  if (tabla == NULL)
    return;

  if (tabla->es_hoja) {
    free(tabla->frames);
    free(tabla->entradas);
  } else {
    for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
      liberar_tabla(tabla->entradas[i]); // recursivo
    }
    free(tabla->entradas);
  }
  free(tabla);
}

void mostrar_bitmap() {//modo debug only
  log_debug(logger_memoria,"\nEstado del bitmap de frames:\n");
  for (int i = 0; i < CANT_FRAMES; i++) {
    log_debug(logger_memoria,"%d", bitmap_frames[i] ? 1 : 0);
  }
  log_debug(logger_memoria,"\n");
}

int asignar_frame_libre() {
  for (int i = 0; i < CANT_FRAMES; i++) {
    if (!bitmap_frames[i]) {
      bitmap_frames[i] = true;
      return i;
    }
  }
  return -1; // no hay frames disponibles
}

void asignar_frames_hojas(TablaPagina *tabla) {
  if (tabla->es_hoja) {
    for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
      if (tabla->frames[i] == -1) {
        int frame = asignar_frame_libre();
        if (frame != -1)
          tabla->frames[i] = frame;
        else {
            log_error(logger_memoria,"No hay más frames disponibles.");
        }
      }
    }
  } else {
    for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
      asignar_frames_hojas(tabla->entradas[i]);
    }
  }
}

Proceso* guardarProceso(Proceso* p,uint32_t PID,uint32_t tam, char* pseudocodigo) {

  p->pid = PID;  
  p->tamanio_reservado = tam;
  p->pseudocodigo = pseudocodigo;
  p->lista_instrucciones = leer_archivo_y_cargar_instrucciones(p->pseudocodigo);
  char* clave = pasarUnsignedAChar(PID);
  agregarADiccionario(diccionarioProcesos,clave,p);
  free(clave);

  

  return p;
}

int *reservar_frames(int cantidad) {
  int *frames = malloc(sizeof(int) * cantidad);
  if (!frames)
    return NULL;

  int encontrados = 0;
  for (int i = 0; i < CANT_FRAMES && encontrados < cantidad; i++) {
    if (!bitmap_frames[i]) {
      bitmap_frames[i] = true;
      frames[encontrados++] = i;
    }
  }

  if (encontrados < cantidad) { //no llego a encontrar los necesarios libero 
   
    for (int j = 0; j < encontrados; j++)
      bitmap_frames[frames[j]] = false;
    free(frames);
    return NULL;
  }


  return frames;
}

int reservar_memoria(Proceso *p, int bytes) {

  int paginas_necesarias = (bytes + TAM_PAGINA - 1) / TAM_PAGINA;
  int *frames = reservar_frames(paginas_necesarias);
  if (!frames) {
    log_debug(logger_memoria, "No hay suficientes frames libres para %d bytes\n", bytes);
    return -1;
  }
    p->tabla_raiz = crear_tabla_nivel(1);
    int pagina_logica_actual = 0;
    asignar_frames_en_tabla(p->tabla_raiz, paginas_necesarias, frames, &pagina_logica_actual, 1, p);

  free(frames);
  return 0;
}



int escribir_memoria(Proceso *p,  int dir_fisica, char *texto) {
  int len = strlen(texto);

  for (int i = 0; i < len; i++) {
    if (dir_fisica == -1) {
     log_error(logger_memoria,"## PID: <%d>  - ERROR: Dirección inválida <%d>", p->pid, dir_fisica + i);
      return -1;
    }

  ((char*)memoria_principal)[dir_fisica + i] = texto[i];
    p->metricas.escrituras_memoria++;
  }

  log_info(logger_memoria, "## PID: <%u> - Escritura - Dir. Física: <%d> - Tamaño: <%d>", p->pid, dir_fisica, len);
  return 1;
}


char leer_byte(Proceso *p, int dir_fisica) {
  if (dir_fisica < 0 || dir_fisica >= CANT_FRAMES * TAM_PAGINA) {
    log_debug(logger_memoria,"## PID: <%d> - ERROR: Lectura fuera de límites físicos", p->pid);
    return -1;
  }

  return ( (char*)memoria_principal)[dir_fisica];
}

char* leer_memoria(Proceso *p, int dir_fisica,int tamanio) {
  
  char* val= malloc(tamanio+1);

  if (!val) {
    log_error(logger_memoria, "Error al reservar memoria para la lectura");
    return strdup(""); 
  }

  for(int i= 0;i < tamanio;i++){
    val[i] = leer_byte(p, dir_fisica + i); 
  }


  val[tamanio] = '\0';

  p->metricas.lecturas_memoria++;


  log_info(logger_memoria, "## PID: <%u> - Lectura - Dir. Física: <%d> - Tamaño: <%d>", p->pid, dir_fisica, tamanio);

  return val;
}



void mostrar_procesos_activos() {
  log_debug(logger_memoria,"\n=== Procesos Activos ===\n");

  t_list* procesos = dictionary_elements(diccionarioProcesos->diccionario);
  for (int i = 0; i < list_size(procesos); i++) {
      Proceso* p = list_get(procesos, i);
      int cant_paginas = (p->tamanio_reservado + TAM_PAGINA - 1) / TAM_PAGINA;
      log_debug(logger_memoria,"PID %d: %d bytes (%d páginas) | Tabla raíz en %p\n",
             p->pid, p->tamanio_reservado, cant_paginas, (void*) p->tabla_raiz);
  }

  list_destroy(procesos);
}


void limpiar_frames_en_tabla(TablaPagina *tabla, int *frames, int cantidad,
                             int *index, int nivel_actual, Proceso *p) {

  p->metricas.accesos_tabla_paginas++;

  for (int i = 0; i < ENTRADAS_POR_TABLA && *index < cantidad; i++) {
    if (tabla->es_hoja) {
      if (tabla->frames[i] == frames[*index]) {
        tabla->frames[i] = -1;
        (*index)++;
      }
    } else {
      limpiar_frames_en_tabla(tabla->entradas[i], frames, cantidad, index,
                              nivel_actual + 1, p);
    }
  }
}

void liberar_frames_en_tabla(TablaPagina *tabla, int nivel_actual) {
  if (tabla->es_hoja) {
    for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
      if (tabla->frames[i] != -1) {
        bitmap_frames[tabla->frames[i]] = false;
        tabla->frames[i] = -1;
      }
    }
  } else {
    for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
      if (tabla->entradas[i]) {
        liberar_frames_en_tabla(tabla->entradas[i], nivel_actual + 1);
      }
    }
  }
}

void liberar_memoria(Proceso *p) {

    if (!p || !p->tabla_raiz){
        return;
    }
  
  liberar_frames_en_tabla(p->tabla_raiz, 1);
  liberar_tabla(p->tabla_raiz);

  p->tabla_raiz = NULL;
  p->tamanio_reservado=0;
}

void destruir_proceso(uint32_t pid) {
  char* clave = pasarUnsignedAChar(pid);
  Proceso *p = sacarDeDiccionario(diccionarioProcesos, clave);
  free(clave);
  log_info(logger_memoria, "## PID: <%u> - Proceso Destruido - Métricas - Acc.T.Pag: <%d>; Inst.Sol.: <%d>; SWAP: <%d>; Mem.Prin.: <%d>; Lec.Mem.: <%d>; Esc.Mem.: <%d>",
    p->pid,
    p->metricas.accesos_tabla_paginas,
    p->metricas.instrucciones_solicitadas,
    p->metricas.bajadas_swap,
    p->metricas.subidas_memoria,
    p->metricas.lecturas_memoria,
    p->metricas.escrituras_memoria);

  liberar_memoria(p); 
  liberar_entrada_swap(p->pid);
  
  list_destroy_and_destroy_elements(p->lista_instrucciones, free);
  free(p->pseudocodigo);
  free(p);



}

void liberar_entrada_swap(int pid) {
  for (int i = 0; i < list_size(tabla_swap); i++) {
      EntradaSwap *entrada = list_get(tabla_swap, i);
      if (entrada->pid == pid) {
          list_remove_and_destroy_element(tabla_swap, i, free);
          log_debug(logger_memoria, "Entrada de swap liberada para PID: <%u>", pid);
          return;
      }
  }
}

bool realizar_dump_memoria(int pid) {
  char* clave = pasarUnsignedAChar(pid);
    Proceso* proceso = leerDeDiccionario(diccionarioProcesos, clave);
    if (!proceso) {
        return false;
    }
    
    dump_memory(proceso);
    free(clave);
    
    return true;
}

void escribir_tabla_en_archivo(FILE *archivo, TablaPagina *tabla, int nivel_actual, int *bytes_escritos, int tam, int *paginas_recorridas, Proceso *p) {
  if (!tabla) return;
  p->metricas.accesos_tabla_paginas++;
  int paginas_reservadas = (tam + TAM_PAGINA - 1) / TAM_PAGINA;

  if (tabla->es_hoja) {
      for (int i = 0; i < ENTRADAS_POR_TABLA && *paginas_recorridas < paginas_reservadas; i++) {
          int frame = tabla->frames[i];

          if (frame != -1) {
            int faltan = tam - *bytes_escritos;
            if (faltan <= 0) break;
        
            int cantidad = (faltan >= TAM_PAGINA) ? TAM_PAGINA : faltan;
        
            fwrite((char*)memoria_principal + frame * TAM_PAGINA, 1, cantidad, archivo);
            log_debug(logger_memoria, "Frame %d: %.*s", frame, cantidad, (char*)memoria_principal + frame * TAM_PAGINA);

            *bytes_escritos += cantidad;
        
            (*paginas_recorridas)++;
            if (*paginas_recorridas >= paginas_reservadas) break;
        }
      }
  } else {
      for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
          escribir_tabla_en_archivo(archivo, tabla->entradas[i], nivel_actual + 1, bytes_escritos, tam, paginas_recorridas, p);
      }
  }
}

void escribir_tabla_en_archivo_dump(FILE *archivo, TablaPagina *tabla, int nivel_actual, int *bytes_escritos, int tam, int *paginas_recorridas, Proceso *p) {
  
  if (!tabla) return;
  p->metricas.accesos_tabla_paginas++;
  int paginas_reservadas = tam / TAM_PAGINA;

  if (tabla->es_hoja) {
      for (int i = 0; i < ENTRADAS_POR_TABLA && *paginas_recorridas < paginas_reservadas; i++) {
          int frame = tabla->frames[i];

          if (frame != -1) {
            fwrite((char*)memoria_principal + frame * TAM_PAGINA, 1, TAM_PAGINA, archivo);
            *bytes_escritos += TAM_PAGINA;
            (*paginas_recorridas)++;
            if (*paginas_recorridas >= paginas_reservadas) break;
        }
      }
  } else {
      for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
          escribir_tabla_en_archivo(archivo, tabla->entradas[i], nivel_actual + 1, bytes_escritos, tam, paginas_recorridas, p);
      }
  }
}

void dump_memory(Proceso *p) {
    // Obtiene el timestamp actual
    time_t timestamp = time(NULL);
    struct tm *tm_info = localtime(&timestamp);
    
    char filename[256];
    strftime(filename, sizeof(filename), "%Y%m%d-%H%M%S", tm_info);
    char full_filename[512];
    snprintf(full_filename, sizeof(full_filename), "%s%d-%s.dmp", dump_path, p->pid, filename);
    
    // Crear directorio si no existe
    char dir_path[512];
    strncpy(dir_path, dump_path, sizeof(dir_path) - 1);
    dir_path[sizeof(dir_path) - 1] = '\0';
    
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';
        mkdir(dir_path, 0755);
    }
    
    FILE *dump_file = fopen(full_filename, "wb");
    if (!dump_file) {
        log_error(logger_memoria, "Error al crear archivo de dump: <%s>", full_filename);
        return;
    }
    
    log_debug(logger_memoria, "## PID: <%d> - Memory Dump - Creando archivo: <%s>", p->pid, full_filename);
    
    int bytes_escritos = 0;
    int paginas_recorridas = 0;
    int paginas_reservadas = (p->tamanio_reservado + TAM_PAGINA - 1) / TAM_PAGINA;
    int tam_reservado = paginas_reservadas * TAM_PAGINA;

    escribir_tabla_en_archivo_dump(dump_file, p->tabla_raiz, 1, &bytes_escritos, tam_reservado, &paginas_recorridas, p);
  
    
    fclose(dump_file);
    
    log_debug(logger_memoria, "## PID: <%d> - Memory Dump completado - <%d> bytes escritos en <%s>", 
             p->pid, bytes_escritos, full_filename);
  



  //if (!p || !p->tabla_raiz) {
  //  log_error(logger_memoria, "No se puede mostrar la tabla raíz del Proceso PID <%d>: no existe o no tiene páginas.\n ", p ? p->pid : -1);
  //  return;
  //} else {
  //  log_debug(logger_memoria,"## PID: %d\n", p->pid);
  //  imprimir_tabla(p->tabla_raiz, 1, 0);
  //}
  //log_debug(logger_memoria,"\n");
  //log_debug(logger_memoria,"MODO DEBUG ONLY:\n");
  //mostrar_bitmap();
  //mostrar_procesos_activos();
}

int guardarProcesoYReservar(uint32_t PID,uint32_t tam, char* pseudocodigo) {
  Proceso *p = malloc(sizeof(Proceso));
  if (!p) {
    log_error(logger_memoria, "Error al crear proceso.");
    exit(EXIT_FAILURE);
  }
  
  if (reservar_memoria(p, tam) < 0) {
   log_error(logger_memoria, "No se pudo asignar memoria al proceso\n");
    free(p);
    mostrar_procesos_activos();
    return -1;
  }
  
  p = guardarProceso(p,PID,tam,pseudocodigo); 


  memset(&p->metricas, 0, sizeof(MetricaProceso));
  log_info(logger_memoria,"## PID: <%u> - Proceso creado - Tamaño: <%u>", p->pid, p->tamanio_reservado);
  //mostrar_bitmap();
return 0;

}

int suspender_proceso(Proceso *p) {


  FILE *archivo_swap = fopen(path_swapfile, "ab");

  if (!archivo_swap) {
   log_error(logger_memoria,"No se pudo abrir swapfile.bin para suspender el proceso <%u>", p->pid);
    return -1;
  }


  uint32_t offset = ftell(archivo_swap);

  EntradaSwap *entrada = malloc(sizeof(EntradaSwap));
  entrada->pid = p->pid;
  entrada->tamanio_original = p->tamanio_reservado;
  entrada->offset = offset;

  int bytes_escritos = 0;
  int paginas_recorridas = 0;

  escribir_tabla_en_archivo(archivo_swap, p->tabla_raiz, 1, &bytes_escritos,  p->tamanio_reservado, &paginas_recorridas,p );


  list_add(tabla_swap, entrada);
  p->metricas.bajadas_swap++;
  fclose(archivo_swap);
  liberar_memoria(p);
  log_debug(logger_memoria, "PID: <%u> - Proceso suspendido correctamente en swapfile", p->pid);
  
  return 1; 
}

int restaurar_proceso(Proceso *p ) {

  EntradaSwap *entrada = NULL;
  for (int i = 0; i < list_size(tabla_swap); i++) {
    EntradaSwap *e = list_get(tabla_swap, i);
    if (e->pid == p->pid) {
      entrada = e;
      break;
    }
  }

  if (!entrada) {
    log_error(logger_memoria, "PID <%u> no está en swap", p->pid);
    return -1;
  }

  FILE *archivo_swap = fopen(path_swapfile, "rb");
  if (!archivo_swap) {
    log_error(logger_memoria, "No se pudo abrir swapfile.bin para restauración");
    return -1;
  }

  fseek(archivo_swap, entrada->offset, SEEK_SET);

  int cantidad_paginas = (entrada->tamanio_original + TAM_PAGINA - 1) / TAM_PAGINA;

  int *frames = reservar_frames(cantidad_paginas);

  if (!frames) {
   log_error(logger_memoria, "No hay frames disponibles para restaurar PID: <%u>", p->pid);
    fclose(archivo_swap);
    return -1;
  }

  p->tabla_raiz = crear_tabla_nivel(1);

  int pagina_logica_actual = 0;
  asignar_frames_en_tabla(p->tabla_raiz, cantidad_paginas, frames, &pagina_logica_actual, 1, p);

  for (int i = 0; i < cantidad_paginas; i++) {
    fread((char*)memoria_principal + frames[i] * TAM_PAGINA, 1, TAM_PAGINA, archivo_swap);
  }

  p->tamanio_reservado = entrada->tamanio_original;

  fclose(archivo_swap);
  free(frames);

  for (int i = 0; i < list_size(tabla_swap); i++) {
    EntradaSwap *e = list_get(tabla_swap, i);
    if (e->pid == p->pid) {
      list_remove_and_destroy_element(tabla_swap, i, free);
      break;
    }
  }

  log_info(logger_memoria," PID: <%u> - Restaurado exitosamente desde swapfile", p->pid);
  p->metricas.subidas_memoria++;
  return 1;
}
