#ifndef IO_H_
#define IO_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils/shared.h"
#include "utils/configs.h"

int socket_io_kernel;
char* ip_kernel;
int puerto_kernel;
char* valor;

t_log* logger_io;
t_config* config_io;
t_log_level log_level;

void leerConfigIO(t_config* config_io);

#endif
