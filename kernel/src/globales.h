#ifndef GLOBALES_H_
#define GLOBALES_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils/configs.h"
#include "utils/conexiones.h"
#include "utils/shared.h"

extern int socket_kernel_memoria;
extern char* archivo_config;
extern char* ip_memoria;
extern char* algoritmo_planificacion;
extern int puerto_memoria;
extern int tiempo_suspension;
extern char* log_level;
extern t_log* logger_kernel;
extern t_config* config_kernel;

#endif