#include "administracion-memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool *bitmap_frames = NULL;
char *memoria_real = NULL;
// Mutexes
t_diccionarioConSemaforos* diccionarioProcesos;



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
}


// Inicializa la memoria simulada y el bitmap de frames libres
void inicializar_memoria() {
  CANT_FRAMES = TAM_MEMORIA / TAM_PAGINA;

  printf("Cantidad de frames: %d\n", CANT_FRAMES);
  printf("Tamaño de pagina: %d\n", TAM_PAGINA);

  memoria_real = malloc(TAM_MEMORIA);
  if (!memoria_real) {
    fprintf(stderr, "ERROR: No se pudo asignar memoria simulada\n");
    exit(EXIT_FAILURE);
  }

  bitmap_frames = malloc(sizeof(bool) * CANT_FRAMES);
  if (!bitmap_frames) {
    fprintf(stderr, "ERROR: No se pudo asignar bitmap de frames\n");
    free(memoria_real);
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < CANT_FRAMES; i++) {
    bitmap_frames[i] = false; // false indica frame libre
  }
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
int traducir_direccion(Proceso *p, int direccion_virtual) {
  int nro_pagina = direccion_virtual / TAM_PAGINA;
  int desplazamiento = direccion_virtual % TAM_PAGINA;

  TablaPagina *tabla = p->tabla_raiz;


  int divisor = 1;
  for (int i = 1; i < CANTIDAD_NIVELES; i++)
    divisor *= ENTRADAS_POR_TABLA;

  for (int nivel = 1; nivel <= CANTIDAD_NIVELES; nivel++) {
    p->metricas.accesos_tabla_paginas++;

    int entrada = (nro_pagina / divisor) % ENTRADAS_POR_TABLA;

    if (nivel == CANTIDAD_NIVELES) {
      int frame = tabla->frames[entrada];
      if (frame == -1) {
        fprintf(stderr, "Error: página no asignada\n");
        return -1;
      }
      return frame * TAM_PAGINA + desplazamiento;
    } else {
      tabla = tabla->entradas[entrada];
      if (!tabla) {
        fprintf(stderr, "Error: tabla intermedia nula\n");
        return -1;
      }
      divisor /= ENTRADAS_POR_TABLA;
    }
  }

  return -1; 
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
    for (int j = 0; j < indent; j++)
      printf("  "); 
    if (tabla->es_hoja) {
      int frame = tabla->frames[i];
      if (frame == -1)
       printf("Nivel %d - Entrada %d: [SIN FRAME]\n", nivel_actual, i);
      else
        printf("Nivel %d - Entrada %d: Frame %d\n", nivel_actual, i, frame);
    } else {
          if (tabla->entradas == NULL) {
            printf("  (entradas == NULL)\n");
            return;
        }
      if(tabla->entradas[i] != NULL){
        printf("Nivel %d - Entrada %d:\n", nivel_actual, i);
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

void mostrar_bitmap() {
  printf("\nEstado del bitmap de frames:\n");
  for (int i = 0; i < CANT_FRAMES; i++) {
    printf("%d", bitmap_frames[i] ? 1 : 0);
  }
  printf("\n");
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
            printf("No hay más frames disponibles\n");
        }
      }
    }
  } else {
    for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
      asignar_frames_hojas(tabla->entradas[i]);
    }
  }
}

Proceso* guardarProceso(uint32_t PID,uint32_t tam, char* pseudocodigo) {
  Proceso *p = malloc(sizeof(Proceso));
  if (!p) {
    fprintf(stderr, "Error al crear proceso\n");
    exit(EXIT_FAILURE);
  }
  p->pid = PID;  
  p->tamanio_reservado = tam;
  agregarADiccionario(diccionarioProcesos,pasarUnsignedAChar(PID),p);
  p->pseudocodigo = pseudocodigo;
  p->lista_instrucciones = leer_archivo_y_cargar_instrucciones(p->pseudocodigo);

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
    fprintf(stderr, "No hay suficientes frames libres para %d bytes\n", bytes);
    return -1;
  }
    p->tabla_raiz = crear_tabla_nivel(1);
    int pagina_logica_actual = 0;
    asignar_frames_en_tabla(p->tabla_raiz, paginas_necesarias, frames, &pagina_logica_actual, 1, p);

    p->tamanio_reservado += paginas_necesarias * TAM_PAGINA;

  free(frames);
  return 0;
}

void escribir_byte(Proceso *p, int direccion_virtual, char valor) {
  int direccion_fisica = traducir_direccion(p, direccion_virtual);
  if (direccion_fisica == -1) {  return;}
  

  memoria_real[direccion_fisica]= valor;
}

void escribir_memoria(Proceso *p, int direccion_virtual, char valor) {
  escribir_byte(p, direccion_virtual, valor);
  p->metricas.escrituras_memoria++;

  printf("→ Proceso %d escribió '%c' en dir virtual %d\n", p->pid, valor,
         direccion_virtual);
}

char leer_byte(Proceso *p, int direccion_virtual) {
  int direccion_fisica = traducir_direccion(p, direccion_virtual);

  if (direccion_fisica == -1) {
    return -1;
  }

  return memoria_real[direccion_fisica];
}

void leer_memoria(Proceso *p, int direccion_virtual) {
  char val = leer_byte(p, direccion_virtual);
  p->metricas.lecturas_memoria++;

  printf("Proceso %d leyó '%c' en dir virtual %d\n", p->pid, val, direccion_virtual);
}

void mostrar_procesos_activos() {
  printf("\n=== Procesos Activos ===\n");

  sem_wait(diccionarioProcesos->semaforoMutex);

  t_list* procesos = dictionary_elements(diccionarioProcesos->diccionario);
  for (int i = 0; i < list_size(procesos); i++) {
      Proceso* p = list_get(procesos, i);
      int cant_paginas = (p->tamanio_reservado + TAM_PAGINA - 1) / TAM_PAGINA;
      printf("PID %d: %d bytes (%d páginas) | Tabla raíz en %p\n",
             p->pid, p->tamanio_reservado, cant_paginas, (void*) p->tabla_raiz);
  }

  list_destroy(procesos);

  sem_post(diccionarioProcesos->semaforoMutex);
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

void destruir_proceso(Proceso *p) {
  liberar_memoria(p); // libera frames y tabla

  sacarDeDiccionario(diccionarioProcesos, pasarUnsignedAChar(p->pid));
    free(p);

    

}

void dump_memory(Proceso *p) {
  mostrar_bitmap();
  imprimir_tabla(p->tabla_raiz, 1, 0);
  mostrar_procesos_activos();
}

int guardarProcesoYReservar(uint32_t PID,uint32_t tam, char* pseudocodigo) {
  Proceso *p = guardarProceso(PID,tam,pseudocodigo);
  if (reservar_memoria(p, tam) < 0) {
    fprintf(stderr, "Error: no se pudo asignar memoria al proceso\n");
    free(p);
    return -1;
  }
  
  memset(&p->metricas, 0, sizeof(MetricaProceso));

  dump_memory(p);

return 0;

}

char **leer_instrucciones(const char *ruta, int *cantidad) {

  printf("Ruta recibida: %s\n", ruta);
  FILE *archivo = fopen(ruta, "r");

  if (!archivo) {
    perror("No se pudo abrir el archivo de instrucciones");
    return NULL;
  }

  char **lineas = NULL;
  char buffer[256];
  int count = 0;

  while (fgets(buffer, sizeof(buffer), archivo)) {
    buffer[strcspn(buffer, "\r\n")] = 0;
    lineas = realloc(lineas, sizeof(char *) * (count + 1));
    lineas[count++] = strdup(buffer);
  }

  fclose(archivo);
  *cantidad = count;
  return lineas;
}
