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

}PCB ;


extern int socket_kernel_memoria;

//CONFIG
extern char* ip_memoria;
extern char* algoritmo_planificacion;
extern int puerto_memoria;
extern int tiempo_suspension;
extern char* log_level;
extern t_log* logger_kernel;
extern int puerto_escucha_dispatch;
extern int puerto_escucha_interrupt;
extern int puerto_escucha_IO;


#endif