#include <syscalls.h>

void dump_memory(uint32_t pid) {
    log_info(logger_kernel, "## (%d) - Solicitó syscall: DUMP_MEMORY", pid);
    int socket_memoria_particular = crear_conexion(logger_kernel, ip_memoria, puerto_memoria);

    t_paquete* paquete = crear_super_paquete(DUMP_MEMORY);

    if (paquete == NULL) {
        log_error(logger_kernel, "Error al crear el paquete para DUMP_MEMORY");
        return;
    }

    cargar_uint32_t_al_super_paquete(paquete, pid);
    enviar_paquete(paquete, socket_memoria_particular);
    eliminar_paquete(paquete);

    op_code respuesta = recibir_operacion(socket_memoria_particular);

    PCB* proceso = NULL;

    sem_wait(listaProcesosReady->semaforoMutex);
    proceso = buscarPCBEnLista(listaProcesosReady->lista, pid);
    sem_post(listaProcesosReady->semaforoMutex);

    if (proceso == NULL) {
        log_error(logger_kernel, "## (%d) - No se encontró el PCB para DUMP_MEMORY", pid);
        return;
    }

    if (respuesta == DUMP_MEMORY_OK) {
        log_info(logger_kernel, "## (%d) - Finalizó correctamente DUMP_MEMORY", pid);
        pasarAReady(proceso);
    } else {
        log_error(logger_kernel, "## (%d) - Error en DUMP_MEMORY. Finalizando proceso", pid);
        pasarAExit(proceso);
    }

    liberar_conexion(socket_memoria_particular);
}

void syscall_io(uint32_t pid, char* nombreIO, uint32_t tiempo) {
    log_info(logger_kernel, "## (%d) - Solicitó syscall: IO", pid);

    PCB* proceso = NULL;

    sem_wait(listaProcesosReady->semaforoMutex);
    proceso = buscarPCBEnLista(listaProcesosReady->lista, pid);
    sem_post(listaProcesosReady->semaforoMutex);

    if (proceso == NULL) {
        log_error(logger_kernel, "## (%d) - No se encontró el PCB para syscall IO", pid);
        return;
    }

    DispositivoIO* dispositivo = NULL;
    sem_wait(listaDispositivosIO->semaforoMutex);
    dispositivo = buscarIOSegunNombre(nombreIO);
    sem_post(listaDispositivosIO->semaforoMutex);

    if (dispositivo == NULL) {
        log_error(logger_kernel, "## (%d) - Dispositivo IO %s no encontrado. Finalizando proceso", pid, nombreIO);
        pasarAExit(proceso);
        return;
    }

    log_info(logger_kernel, "## (%d) - Bloqueado por IO: %s", pid, nombreIO);

    pasarABLoqueadoEIniciarContador(proceso, tiempo, nombreIO);
}

PCB* buscarPCBEnLista(t_list* lista, uint32_t pid) {
    bool _mismoPID(PCB* pcb) {
        return pcb->PID == pid;
    }

    return list_find(lista, (void*)_mismoPID);
}

