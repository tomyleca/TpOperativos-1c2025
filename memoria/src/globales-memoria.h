#ifndef GLOBALES_MEMORIA_H_
#define GLOBALES_MEMORIA_H_

#include "utils/conexiones.h"
#include "utils/configs.h"
#include "utils/shared.h"


#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>

#include <pthread.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>

#define ERROR_MEMORIA_INSUFICIENTE -1
#define OK 0
#define ERROR_CREACION_ESTRUCTURAS -2
#define SOLICITAR_MEMORIA_PROCESO 1
typedef struct {
    uint32_t ax;            // Registro AX
    uint32_t bx;            // Registro BX
    uint32_t cx;            // Registro CX
    uint32_t dx;            // Registro DX
    uint32_t ex;            // Registro EX
    uint32_t fx;            // Registro FX
    uint32_t gx;            // Registro GX
    uint32_t hx;            // Registro HX
    uint32_t pc;            // Program Counter
    char* pseudocodigo;     // Archivo de pseudocodigo de cada tid
    t_list* instrucciones;  // lista de instrucciones 
} t_pids;

typedef struct {
    int pid; // Identificador del proceso
    int tamanio_proceso;
    t_pids datos_pid;
    int id_tabla_primer_nivel; // Identificador de la tabla de primer nivel
} t_contexto;


// Estructura para la información que envía Kernel 
typedef struct {
    int pid; 
    char* archivo_pseudocodigo; 
    int tamanio_proceso; 
} t_info_kernel; // Aca van los datos del proceso que nos manda Kernel

// Estructura para las instrucciones
typedef struct{
	char* mnemonico;
    char* primero_parametro;
    char* segundo_parametro;
    char* tercero_parametro;
} t_instruccion_codigo;

extern t_contexto* nuevo_contexto_provisorio;
extern t_contexto* nuevo_contexto;


extern int conexion;
extern char* ip;
extern char* puerto;
extern char* valor;
extern int fd_escucha_servidor;

extern t_log* logger_memoria;
extern t_config* config_memoria;

extern char* puerto_escucha;
extern int tam_memoria;
extern int tam_pagina;
extern int entradas_por_tabla;
extern int cant_niveles;
extern int retardo_memoria;
extern char* path_swapfile;
extern int retardo_swap;
extern t_log_level log_level;
extern char* dump_path;
extern t_list* lista_contextos;


#endif