#ifndef GLOBALES_MEMORIA_H_
#define GLOBALES_MEMORIA_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include<readline/readline.h>
#include "utils/shared.h"
#include "utils/conexiones.h"
#include "utils/configs.h"


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