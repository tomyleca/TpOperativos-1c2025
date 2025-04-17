#ifndef GLOBALES_H_
#define GLOBALES_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>

typedef struct {
int PID;
int PC;
int ME[6];
int MT[6];
char* archivoPseudocodigo;
unsigned int tam;
} PCB;

typedef enum{
    NEW,
    READY,
    BLOCKED,
    SUSPENDIDO_BLOCKED,
    SUSPENDIDO,
    EXECUTE,
    EXIT
} ESTADO;

//HILOS
extern pthread_t hilo_escuchar_kernel;
extern pthread_t hilo_escuchar_kernel_interrupcion;
extern pthread_t hilo_conectar_kernel_memoria;
extern pthread_t hilo_crear_kernel_memoria;


extern int socket_kernel_memoria;
extern int socket_kernel_io;
extern int cliente_kernel;
extern int socket_kernel_cpu_dispatch;
extern int socket_kernel_cpu_interrupt;
extern int cliente_kernel_dispatch;
extern int cliente_kernel_interrupt;

//CONFIG Y LOGGER
extern char* ip_memoria;
extern char* algoritmo_planificacion;
extern int puerto_memoria;
extern int tiempo_suspension;
extern t_log_level log_level;
extern t_log* logger_kernel;
extern int puerto_escucha_dispatch;
extern int puerto_escucha_interrupt;
extern int puerto_escucha_IO;

//PROCESOS




#endif