#ifndef GLOBALES_IO_H_
#define GLOBALES_IO_H_

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

extern int socket_io_kernel;
extern char* ip_kernel;
extern int puerto_kernel;
extern char* valor;

extern t_log* logger_io;
extern t_config* config_io;
extern t_log_level log_level;

#endif