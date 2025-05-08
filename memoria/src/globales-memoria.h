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

extern int conexion;
extern char* ip;
extern char* puerto;
extern char* valor;

extern t_log* logger_memoria;
extern t_config* config_memoria;

extern int puerto_escucha;
extern int tam_memoria;
extern int tam_pagina;
extern int entradas_por_tabla;
extern int cant_niveles;
extern int retardo_memoria;
extern char* path_swapfile;
extern int retardo_swap;
extern t_log_level log_level;
extern char* dump_path;

#endif