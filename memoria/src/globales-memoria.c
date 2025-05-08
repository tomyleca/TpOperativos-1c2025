#include "globales-memoria.h"

int conexion;
char* ip;
char* puerto;
char* valor;

t_log* logger_memoria;
t_config* config_memoria;

int puerto_escucha;
int tam_memoria;
int tam_pagina;
int entradas_por_tabla;
int cant_niveles;
int retardo_memoria;
char* path_swapfile;
int retardo_swap;
t_log_level log_level;
char* dump_path;
