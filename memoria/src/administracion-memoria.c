#include "administracion-memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int proximo_pid = 0;
bool *bitmap_frames = NULL;
char *memoria_real = NULL;
Proceso **Procesos = NULL;
int cantidad_Procesos = 0;
TablaPagina *tabla_raiz = NULL;
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
    tabla->entradas = malloc(sizeof(TablaPagina *) * ENTRADAS_POR_TABLA);
    for (int i = 0; i < ENTRADAS_POR_TABLA; i++){
        tabla->entradas[i] = crear_tabla_nivel(nivel_actual + 1);
    }
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
      asignar_frames_en_tabla(tabla->entradas[i], paginas_a_reservar, frames, pagina_logica_actual, nivel_actual + 1, p);
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
      printf("Nivel %d - Entrada %d:\n", nivel_actual, i);
      imprimir_tabla(tabla->entradas[i], nivel_actual + 1, indent + 1);
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
  p->cant_paginas = 0;
  p->frames = NULL;
  p->tamanio_reservado = 0;

  
  agregarADiccionario(diccionarioProcesos,pasarUnsignedAChar(PID),p);

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

  int pagina_base = p->cant_paginas;
  p->frames = realloc(p->frames, sizeof(int) * (p->cant_paginas + paginas_necesarias));
  for (int i = 0; i < paginas_necesarias; i++){
    p->frames[p->cant_paginas++] = frames[i];
  }


  int pagina_logica_actual = 0;
  asignar_frames_en_tabla(tabla_raiz, paginas_necesarias, frames, &pagina_logica_actual, 1, p);

  free(frames);
  return pagina_base;
}

void escribir_byte(Proceso *p, int direccion_virtual, char valor) {
  int pagina_logica = direccion_virtual / TAM_PAGINA;
  int offset = direccion_virtual % TAM_PAGINA;

  if (pagina_logica >= p->cant_paginas) {
    fprintf(stderr, "Segmentation fault: dirección fuera del rango asignado\n");
    return;
  }

  int frame = p->frames[pagina_logica];
  int direccion_fisica = frame * TAM_PAGINA + offset;

  memoria_real[direccion_fisica] = valor;
}

void escribir_memoria(Proceso *p, int direccion_virtual, char valor) {
  escribir_byte(p, direccion_virtual, valor);
  p->metricas.escrituras_memoria++;

  printf("→ Proceso %d escribió '%c' en dir virtual %d\n", p->pid, valor,
         direccion_virtual);
}

char leer_byte(Proceso *p, int direccion_virtual) {
  int pagina_logica = direccion_virtual / TAM_PAGINA;
  int offset = direccion_virtual % TAM_PAGINA;

  if (pagina_logica >= p->cant_paginas) {
    fprintf(stderr, "Segmentation fault: acceso fuera de rango\n");
    return -1;
  }

  int frame = p->frames[pagina_logica];
  int direccion_fisica = frame * TAM_PAGINA + offset;
  return memoria_real[direccion_fisica];
}

void leer_memoria(Proceso *p, int direccion_virtual) {
  char val = leer_byte(p, direccion_virtual);
  p->metricas.lecturas_memoria++;

  printf("← Proceso %d leyó '%c' en dir virtual %d\n", p->pid, val,
         direccion_virtual);
}

void mostrar_procesos_activos() {
  printf("\n=== Procesos Activos ===\n");
  for (int i = 0; i < cantidad_Procesos; i++) {
    Proceso *p = Procesos[i];
    printf("PID %d: %d bytes (%d páginas) → Frames: ", p->pid,
           p->tamanio_reservado, p->cant_paginas);
    for (int j = 0; j < p->cant_paginas; j++) {
      printf("%d ", p->frames[j]);
    }
    printf("\n");
  }
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

void liberar_memoria(Proceso *p, int pagina_base, int cantidad) {
  if (pagina_base + cantidad > p->cant_paginas) {
    fprintf(stderr, "Error: rango inválido para liberar\n");
    return;
  }

  int *a_liberar = malloc(sizeof(int) * cantidad);
  for (int i = 0; i < cantidad; i++) {
    int frame = p->frames[pagina_base + i];
    a_liberar[i] = frame;
    bitmap_frames[frame] = false;
  }

  int idx = 0;
  limpiar_frames_en_tabla(tabla_raiz, a_liberar, cantidad, &idx, 1, p);

  for (int i = pagina_base + cantidad; i < p->cant_paginas; i++) {
    p->frames[i - cantidad] = p->frames[i];
  }

  p->cant_paginas -= cantidad;
  p->frames = realloc(p->frames, sizeof(int) * p->cant_paginas);
  p->tamanio_reservado -= cantidad * TAM_PAGINA;

  free(a_liberar);
}

void destruirProceso(uint32_t PID) {
  Proceso* p = sacarDeDiccionario(diccionarioProcesos,pasarUnsignedAChar(PID));
  liberar_memoria(p, 0, p->cant_paginas);

  free(p->frames);
  free(p);
}

Proceso *guardarProcesoYReservar(uint32_t PID,uint32_t tam, char* pseudocodigo) {
  Proceso *p = guardarProceso(PID,tam,pseudocodigo);
  if (reservar_memoria(p, tam) < 0) {
    fprintf(stderr, "Error: no se pudo asignar memoria al proceso\n");
    free(p);
    return NULL;
  }

 
  memset(&p->metricas, 0, sizeof(MetricaProceso));
  p->tamanio_reservado = tam;

  return p;
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

void interpretar_instruccion(char *linea) {
  char instruccion[32];
  sscanf(linea, "%s", instruccion);

  //TODO: ACA ESTAMOS ASUMIENDO QUE LAS INSTRUCCIONES TIENE UN SENTIDO Y ORDEN LOGICO
  //TODO: VERIFICAR SI PUEDE ELIMINAR OTRO PROCESO.

  if (strcmp(instruccion, "INIT_PROC") == 0) {
    char nombre[32];
    int tamanio;
    sscanf(linea, "INIT_PROC %s %d", nombre, &tamanio);
    //crear_proceso_y_reservar(nombre, tamanio);  COMENTADO PARA PROBAR

    //Todo: si falla que hacemos?


  } else if (strcmp(instruccion, "WRITE") == 0) {
    int dir;
    char valor[256];
    sscanf(linea, "WRITE %d %s", &dir, valor);
    int len = strlen (valor);
    for (int i = 0; i < len; i++){
        escribir_memoria(Procesos[cantidad_Procesos - 1], dir+i, valor[i]);
    }
  } else if (strcmp(instruccion, "READ") == 0) {
    int dir, size;
    sscanf(linea, "READ %d %d", &dir, &size);
    for (int i = 0; i < size; i++){
        leer_memoria(Procesos[cantidad_Procesos - 1], dir + i);
    }
  } else if (strcmp(instruccion, "IO") == 0) {
    // simular espera con sleep o impresión
    int tiempo;
    char recurso[32];
    sscanf(linea, "IO %s %d", recurso, &tiempo);
    printf("Simulando IO en %s por %d ms\n", recurso, tiempo);

  } else if (strcmp(instruccion, "DUMP_MEMORY") == 0) {
    mostrar_bitmap();
    imprimir_tabla(tabla_raiz, 1, 0);
    mostrar_procesos_activos();

  } else if (strcmp(instruccion, "EXIT") == 0) {
    if (cantidad_Procesos > 0) {
      //destruir_proceso(Procesos[cantidad_Procesos - 1]); // último creado COMENTADO PARA PROBAR
      printf(">>> Proceso finalizado.\n");
      mostrar_bitmap();//TODO: Borrar esto es testing
    } else {
      printf(">>> No hay proceso activo para finalizar.\n");
    }

  } else if (strcmp(instruccion, "NOOP") == 0 ||
             strcmp(instruccion, "GOTO") == 0) {
    // ignorar o loguear

  } else {
    printf("Instrucción desconocida: %s\n", instruccion);
  }
}