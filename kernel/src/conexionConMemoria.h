#ifndef CONEXION_CON_MEMORIA_H_
#define CONEXION_CON_MEMORIA_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "globalesKernel.h"
#include "../../utils/src/utils/shared.h"
#include "../../utils/src/utils/conexiones.h"

// Funciones de conexión con memoria
int crear_conexion_memoria();
void cerrar_conexion_memoria(int socket);
bool inicializar_proceso_memoria(uint32_t pid, uint32_t tam);
bool suspender_proceso_memoria(uint32_t pid);
bool des_suspender_proceso_memoria(uint32_t pid);
bool finalizar_proceso_memoria(uint32_t pid);
bool solicitar_dump_memoria(uint32_t pid);

#endif 