#ifndef CLIENTE_H_
#define CLIENTE_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "globales.h"


int cliente_kernel_memoria(char* ip, char* puerto);
void leer_archivo_config(char* archivo_config);


#endif