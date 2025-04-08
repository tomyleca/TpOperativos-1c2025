#ifndef CPU_H_
#define CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils/shared.h"

int conexion;
char* ip;
char* puerto;
char* valor;

t_log* logger_cpu;
t_config* config_cpu;

#endif
