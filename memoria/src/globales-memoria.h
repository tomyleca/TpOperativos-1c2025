#ifndef GLOBALES_MEMORIA_H_
#define GLOBALES_MEMORIA_H_

#include "utils/conexiones.h"
#include "utils/configs.h"
#include "utils/shared.h"


#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <semaphore.h>
#include <pthread.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>

#define OK 0
typedef struct {
    uint32_t pc;            // Program Counter
    char* pseudocodigo;     // Archivo de pseudocodigo de cada tid
    t_list* instrucciones;  // lista de instrucciones 
} t_pids;

typedef struct {
    uint32_t pid; // Identificador del proceso
    uint32_t tamanio_proceso;
    t_pids datos_pid;
    int id_tabla_primer_nivel; // Identificador de la tabla de primer nivel
} t_contexto;


// Estructura para la información que envía Kernel 
typedef struct {
    uint32_t pid; 
    char* archivo_pseudocodigo; 
    uint32_t tamanio_proceso; 
} t_info_kernel; // Aca van los datos del proceso que nos manda Kernel

// Estructura para las instrucciones
typedef struct{
	char* mnemonico;
    char* primero_parametro;
    char* segundo_parametro;
    char* tercero_parametro;
} t_instruccion_codigo;

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
  char* pseudocodigo;
  TablaPagina *tabla_raiz;
  t_list* lista_instrucciones;
  uint32_t pc;
} Proceso;


extern int conexion;
extern char* ip;
extern char* puerto;
extern char* valor;
extern int fd_escucha_servidor;

extern sem_t sem_mutex_memoria;

extern t_log* logger_memoria;
extern t_config* config_memoria;

extern int entrada_nivel_X;
extern char* puerto_escucha;
extern int TAM_MEMORIA;
extern int TAM_PAGINA;
extern int ENTRADAS_POR_TABLA;
extern int CANTIDAD_NIVELES;
extern int CANT_FRAMES;

extern int retardo_memoria;
extern char* path_swapfile;
extern int retardo_swap;
extern t_log_level log_level;
extern char* dump_path;
extern t_list* lista_contextos;
extern pthread_t hilo_memoria;
extern char* path_pseudocodigos;



#endif