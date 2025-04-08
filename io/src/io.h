#ifndef IO_H_
#define IO_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils/shared.h"
#include "utils/config.h"

int socket_io_kernel;
char* ip;
char* puerto;
char* valor;

t_log* logger_io;
t_config* config_io;

#endif
