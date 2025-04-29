#ifndef CONEXION_MEMORIA_H_
#define CONEXION_MEMORIA_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils/shared.h"
#include "utils/conexiones.h"
#include "utils/configs.h"
#include "GlobalesMemoria.h"
#include "funcionesMemoria.h"

void leerConfigMemoria(t_config* config_memoria);

void server_escucha(int fd_escucha_servidor,t_log* memoria_logger);

int atender_cliente(int *fd_conexion);





#endif