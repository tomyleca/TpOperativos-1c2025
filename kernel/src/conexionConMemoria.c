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
