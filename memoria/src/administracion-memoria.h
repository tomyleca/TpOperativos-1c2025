#ifndef ADMINISTRACION_MEMORIA_H_
#define ADMINISTRACION_MEMORIA_H_


#include "globales-memoria.h"
#include "utils/conexiones.h"
#include "utils/configs.h"
#include "utils/shared.h"
#include "utils/monitoresDiccionarios.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

// =====================
// Configuración de memoria
// =====================


extern bool *bitmap_frames;
extern char *memoria_real;

// =====================
// Estructuras de datos
// =====================

typedef struct {
  int accesos_tabla_paginas;
  int instrucciones_solicitadas;
  int bajadas_swap;
  int subidas_memoria;
  int lecturas_memoria;
  int escrituras_memoria;
} MetricaProceso;

typedef struct TablaPagina {
  struct TablaPagina **entradas;
  int *frames;
  int es_hoja;
} TablaPagina;
typedef struct {
  int pid; // ID único
  MetricaProceso metricas;  
  int tamanio_reservado ; // en bytes
  int cantidad_instrucciones;
  TablaPagina *tabla_raiz;
} Proceso;

extern t_diccionarioConSemaforos* diccionarioProcesos;

extern int cantidad_Procesos;




// =====================
// Prototipos
// =====================
void leerConfigMemoria(t_config* config_memoria);
void inicializar_memoria();
TablaPagina *crear_tabla_nivel(int nivel_actual);
void imprimir_tabla(TablaPagina *tabla, int nivel, int indent);
void liberar_tabla(TablaPagina *tabla);
void mostrar_bitmap();
int asignar_frame_libre();
void asignar_frames_hojas(TablaPagina *tabla);

//* interaccion con memoria
Proceso* guardarProceso(uint32_t PID,uint32_t tam, char* pseudocodigo);
int *reservar_frames(int cantidad);
int reservar_memoria(Proceso *p, int bytes);
void escribir_byte(Proceso *p, int direccion_virtual, char valor);
void escribir_memoria(Proceso *p, int direccion_virtual, char valor);
char leer_byte(Proceso *p, int direccion_virtual);
void leer_memoria(Proceso *p, int direccion_virtual);
Proceso *guardarProcesoYReservar(uint32_t PID,uint32_t tam, char* pseudocodigo);
void mostrar_procesos_activos();
void liberar_memoria(Proceso *p);
void destruir_proceso(Proceso *p);
char **leer_instrucciones(const char *ruta, int *cantidad);
void interpretar_instruccion(char *linea);

void dump_memory (Proceso *p);

#endif

