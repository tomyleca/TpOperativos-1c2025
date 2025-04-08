#ifndef CONFIGS_H_
#define CONFIGS_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>


t_config* iniciar_config(char* archivo_config);

t_log *iniciar_logger(char *nombreDelModulo, char* nombre_proceso);


#endif
