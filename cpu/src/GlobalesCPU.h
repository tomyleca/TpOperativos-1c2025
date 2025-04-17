#ifndef GLOBALES_CPU_H_
#define GLOBALES_CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils/shared.h"
#include "utils/conexiones.h"
#include "utils/configs.h"

typedef struct 
{
    int pid;
    t_registros registros;
} t_contexto_cpu;


extern int socket_cpu_memoria;
extern int socket_cpu_kernel_dispatch;
extern int socket_cpu_kernel_interrupt;
extern int fd_cpu_kernel_dispatch;
extern int fd_cpu_kernel_interrupt;

// Config
extern char* ip;
extern char* puerto;
extern char* valor;
extern char* instruccion_recibida;

//HILOS
extern pthread_t hilo_escuchar_kernel_interrupcion;
extern pthread_t hilo_escuchar_kernel;
extern pthread_t hilo_escuchar_memoria;
extern pthread_t hilo_interpretar_instruccion;

//DICTIONARYS

extern t_dictionary* registros;
extern t_dictionary* instrucciones;

extern t_contexto_cpu* contexto;


extern t_log* logger_cpu;
extern t_config* config_cpu;

// Conexiones a módulos
extern char* ip_memoria;
extern int puerto_memoria;
extern char* ip_kernel;
extern int puerto_kernel_dispatch;
extern int puerto_kernel_interrupt;

// TLB
extern int entradas_tlb;
extern char* reemplazo_tlb;

// Caché
extern int entradas_cache;
extern char* reemplazo_cache;
extern int retardo_cache;

// Log level
extern t_log_level log_level;

#endif