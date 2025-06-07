#ifndef CONEXION_CON_MEMORIA_H_
#define CONEXION_CON_MEMORIA_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "globalesKernel.h"
#include "../../utils/src/utils/shared.h"
#include "../../utils/src/utils/conexiones.h"
int crear_conexion_memoria();
void cerrar_conexion_memoria(int socket);
#endif 