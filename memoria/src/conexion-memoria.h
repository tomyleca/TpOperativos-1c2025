#ifndef CONEXION_MEMORIA_H_
#define CONEXION_MEMORIA_H_

#include "GlobalesMemoria.h"
#include "utils/conexiones.h"
#include "utils/configs.h"
#include "utils/shared.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>



void server_escucha(int fd_escucha_servidor,t_log* memoria_logger);
int atender_cliente(int *fd_conexion);

#endif