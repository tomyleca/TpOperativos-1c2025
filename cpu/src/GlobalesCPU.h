#ifndef GLOBALES_CPU_H_
#define GLOBALES_CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <semaphore.h>
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
extern char* identificador_cpu;


// Config
extern char* ip;
extern char* puerto;
extern char* valor;
extern char* instruccion_recibida;
extern t_log* logger_cpu;
extern t_config* config_cpu;

//HILOS
extern pthread_t hilo_escuchar_kernel_interrupcion;
extern pthread_t hilo_escuchar_kernel;
extern pthread_t hilo_crear_kernel_memoria;
extern pthread_t hilo_escuchar_memoria;
extern pthread_t hilo_interpretar_instruccion;

//DICTIONARYS

extern t_dictionary* registros;
extern t_dictionary* instrucciones;

extern t_contexto_cpu* contexto;

//SEMAFOROS
extern sem_t sem_hay_instruccion;
extern sem_t sem_pid;
extern sem_t sem_contexto;


// Conexiones a módulos
extern char* ip_memoria;
extern char* puerto_memoria;
extern char* ip_kernel;
extern char* puerto_kernel_dispatch;
extern char* puerto_kernel_interrupt;

//OTROS
extern bool flag_interrupcion;
extern op_code motivo_interrupcion;
extern int valor_inicial;

//MMU
extern int cant_entradas_tabla; 
extern int tamanio_pagina;              
extern int cant_niveles;

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