#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "globales.h"
#include "cliente.h"

int socket_kernel_memoria;
char* archivo_config;
char* ip_memoria;
char* algoritmo_planificacion;
int puerto_memoria;
int tiempo_suspension;
char* log_level;

t_log* logger_kernel;
t_config* config_kernel;


#endif
