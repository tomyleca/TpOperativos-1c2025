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

bool suspender_proceso_memoria(uint32_t pid) {
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

    int valorSemaforo;
    sem_getvalue(semaforoInicializarProceso,&valorSemaforo);
    

    if(valorSemaforo<=0) 
        sem_post(semaforoInicializarProceso);


    return respuesta == SWAP_SUSPENDER_PROCESO;
   
}

bool des_suspender_proceso_memoria(uint32_t pid) {
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

    if(respuesta == SWAP_ERROR)
    {
        log_error(loggerKernel,"## (<%u>) No hay suficiente memoria para alojar el proceso",pid);
    }

    return respuesta == SWAP_OK;
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
    
    if (respuesta == DUMP_MEMORY_OK) {
        // Recibir el PID de confirmación
        t_buffer* buffer = recibiendo_super_paquete(socket_memoria);
        uint32_t pid_confirmado = recibir_uint32_t_del_buffer(buffer);
        limpiarBuffer(buffer);
        
        char* PIDComoChar = pasarUnsignedAChar(pid_confirmado);
        ProcesoEnEsperaDump* procesoEsperandoDump = leerDeDiccionario(diccionarioProcesosEsperandoDump, PIDComoChar);
        free(PIDComoChar);
        
        if (procesoEsperandoDump != NULL) {
            sem_post(procesoEsperandoDump->semaforoDumpFinalizado);
        }
        
        cerrar_conexion_memoria(socket_memoria);
        return true;
    } else if (respuesta == DUMP_MEMORY_ERROR) {
        // Recibir el PID de error
        t_buffer* buffer = recibiendo_super_paquete(socket_memoria);
        uint32_t pid_error = recibir_uint32_t_del_buffer(buffer);
        limpiarBuffer(buffer);
        
        char* PIDComoChar = pasarUnsignedAChar(pid_error);
        ProcesoEnEsperaDump* procesoEsperandoDump = leerDeDiccionario(diccionarioProcesosEsperandoDump, PIDComoChar);
        free(PIDComoChar);
        
        if (procesoEsperandoDump != NULL) {
            sem_post(procesoEsperandoDump->semaforoDumpFinalizado);
        }
        
        cerrar_conexion_memoria(socket_memoria);
        return false;
    }
    
    cerrar_conexion_memoria(socket_memoria);
    return false;
}

void* avisarFinDeProcesoAMemoria(void* PIDPuntero)
{
    uint32_t PID =  *(uint32_t*)PIDPuntero;
    free(PIDPuntero);
    int fdMemoria = crear_conexion_memoria();
    t_paquete* paquete = crear_super_paquete(FINALIZA_PROCESO);
    cargar_uint32_t_al_super_paquete(paquete,PID);
    enviar_paquete(paquete,fdMemoria);
    eliminar_paquete(paquete);
    recibir_operacion(fdMemoria);
    //if(respuesta!=1)
        //log_error(loggerKernel,"#<%u> Error en la comunicación con memoria al finalizar el proceso",PID);
    int valorSemaforo;
    sem_getvalue(semaforoInicializarProceso,&valorSemaforo);
    

    if(valorSemaforo<=0) 
        sem_post(semaforoInicializarProceso);
    
    cerrar_conexion_memoria(fdMemoria);
    return NULL;
}