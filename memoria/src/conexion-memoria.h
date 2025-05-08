#ifndef CONEXION_MEMORIA_H_
#define CONEXION_MEMORIA_H_

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "utils/conexiones.h"
#include "utils/configs.h"


#include "funciones-memoria.h"
#include "utils/shared.h"
#include "globales-memoria.h"
#include "administracion-memoria.h"


extern t_log* memoria_logger;

bool crear_estructuras_proceso(int pid, int paginas_necesarias);
void server_escucha(int fd_escucha_servidor,t_log* memoria_logger);
int atender_cliente(int *fd_conexion);

#endif