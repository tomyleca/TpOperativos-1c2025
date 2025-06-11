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

bool inicializar_proceso_memoria(uint32_t pid, uint32_t tam) {
    int socket_memoria = crear_conexion_memoria();
    if (socket_memoria == -1) return false;

    t_paquete* paquete = crear_super_paquete(INICIALIZAR_PROCESO);
    if (paquete == NULL) {
        log_error(loggerKernel, "Error al crear paquete para inicializar proceso");
        cerrar_conexion_memoria(socket_memoria);
        return false;
    }

    cargar_uint32_t_al_super_paquete(paquete, pid);
    cargar_uint32_t_al_super_paquete(paquete, tam);
    
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    op_code respuesta = recibir_operacion(socket_memoria);
    cerrar_conexion_memoria(socket_memoria);

    return respuesta == INICIALIZAR_PROCESO;
}

bool suspender_proceso_memoria(uint32_t pid) {
    int socket_memoria = crear_conexion_memoria();
    if (socket_memoria == -1) return false;

    t_paquete* paquete = crear_super_paquete(SUSPENDER_PROCESO);
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

    return respuesta == SUSPENDER_PROCESO;
}

bool des_suspender_proceso_memoria(uint32_t pid) {
    int socket_memoria = crear_conexion_memoria();
    if (socket_memoria == -1) return false;

    t_paquete* paquete = crear_super_paquete(DES_SUSPENDER_PROCESO);
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

    return respuesta == DES_SUSPENDER_PROCESO;
}

bool finalizar_proceso_memoria(uint32_t pid) {
    int socket_memoria = crear_conexion_memoria();
    if (socket_memoria == -1) return false;

    t_paquete* paquete = crear_super_paquete(FINALIZAR_PROCESO);
    if (paquete == NULL) {
        log_error(loggerKernel, "Error al crear paquete para finalizar proceso");
        cerrar_conexion_memoria(socket_memoria);
        return false;
    }

    cargar_uint32_t_al_super_paquete(paquete, pid);
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);

    op_code respuesta = recibir_operacion(socket_memoria);
    cerrar_conexion_memoria(socket_memoria);

    return respuesta == FINALIZAR_PROCESO;
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