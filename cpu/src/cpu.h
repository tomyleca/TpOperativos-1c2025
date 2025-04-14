#ifndef CPU_H_
#define CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "../../utils/src/utils/configs.h"
#include "../../utils/src/utils/shared.h"

int conexion;
char* ip;
char* puerto;
char* valor;

t_log* logger_cpu;
t_config* config_cpu;

char* ip_memoria;
int puerto_memoria;
char* ip_kernel;
int puerto_kernel_dispatch;
int puerto_kernel_interrupt;
int entradas_tlb;
char* reemplazo_tlb;
int entradas_cache;
char* reemplazo_cache;
int retardo_cache;
t_log_level log_level;


#endif
