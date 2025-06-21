#include "conexionConMemoria.h"

int crear_conexion_memoria() {
    int socket_memoria = crear_conexion(loggerKernel, ip_memoria, puerto_memoria);
    if (socket_memoria == -1) {
        log_error(loggerKernel, "Error al crear conexión con memoria");
        return -1;
    }
    return socket_memoria;
}

void cerrar_conexion_memoria(int socket) {
    if (socket != -1) {
        liberar_conexion(socket);
    }
}

bool solicitar_suspender_proceso_memoria(uint32_t pid) {
    int socket_memoria = crear_conexion_memoria();
    if (socket_memoria == -1) return false;

    t_paquete* paquete = crear_super_paquete(SWAP_SUSPENDER_PROCESO);
    if (paquete == NULL) {
        log_error(loggerKernel, "Error al crear paquete para suspender proceso");
        cerrar_conexion_memoria(socket_memoria);
        return false;
    }

    cargar_uint32_t_al_super_paquete(paquete, pid);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    op_code respuesta = recibir_operacion(socket_memoria);
    cerrar_conexion_memoria(socket_memoria);

    return respuesta == SWAP_SUSPENDER_PROCESO;
}

bool solicitar_restaurar_proceso_memoria(uint32_t pid) {
    int socket_memoria = crear_conexion_memoria();
    if (socket_memoria == -1) return false;

    t_paquete* paquete = crear_super_paquete(SWAP_RESTAURAR_PROCESO);
    if (paquete == NULL) {
        log_error(loggerKernel, "Error al crear paquete para des-suspender proceso");
        cerrar_conexion_memoria(socket_memoria);
        return false;
    }

    cargar_uint32_t_al_super_paquete(paquete, pid);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    op_code respuesta = recibir_operacion(socket_memoria);
    cerrar_conexion_memoria(socket_memoria);

    return respuesta == SWAP_RESTAURAR_PROCESO;
}

bool solicitar_dump_memoria(uint32_t pid) {
    int socket_memoria = crear_conexion_memoria();
    if (socket_memoria == -1) return false;

    t_paquete* paquete = crear_super_paquete(DUMP_MEMORY);
    if (paquete == NULL) {
        log_error(loggerKernel, "Error al crear paquete para dump memory");
        cerrar_conexion_memoria(socket_memoria);
        return false;
    }

    cargar_uint32_t_al_super_paquete(paquete, pid);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    op_code respuesta = recibir_operacion(socket_memoria);
    cerrar_conexion_memoria(socket_memoria);

    return respuesta == DUMP_MEMORY_OK;
} 