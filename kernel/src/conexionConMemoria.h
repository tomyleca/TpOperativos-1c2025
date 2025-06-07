#ifndef CONEXION_CON_MEMORIA_H_
#define CONEXION_CON_MEMORIA_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "globalesKernel.h"
#include "../../utils/src/utils/shared.h"
#include "../../utils/src/utils/conexiones.h"

// Operaciones de memoria
typedef enum {
    INICIALIZAR_PROCESO,
    SUSPENDER_PROCESO,
    DES_SUSPENDER_PROCESO,
    FINALIZAR_PROCESO,
    DUMP_MEMORY,
    DUMP_MEMORY_OK,
    DUMP_MEMORY_ERROR
} op_code_memoria;

// Funciones de conexión con memoria
int crear_conexion_memoria();
void cerrar_conexion_memoria(int socket);
bool inicializar_proceso_memoria(uint32_t pid, uint32_t tam);
bool suspender_proceso_memoria(uint32_t pid);
#endif 