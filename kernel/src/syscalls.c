#include <syscalls.h>
#include "conexionConMemoria.h"

void INIT_PROC(char* archivoPseudocodigo,uint32_t tam){
    //Creo un nuevo proceso
    PCB* nuevoProceso=malloc(sizeof(PCB));
    nuevoProceso->archivoPseudocodigo=strdup(archivoPseudocodigo);
    nuevoProceso->tam=tam;
    nuevoProceso->PC=0;
    
    nuevoProceso->PID=pidDisponible;
    
    sem_wait(semaforoPIDDisponible);
        pidDisponible++;
    sem_post(semaforoPIDDisponible);

    

    for(int i=0;i<7;i++)
    {
        nuevoProceso->ME[i]=0;
        nuevoProceso->MT[i]=0;
        nuevoProceso->cronometros[i]=temporal_create();
        temporal_stop(nuevoProceso->cronometros[i]);
    }


    

    nuevoProceso->estimadoRafagaAnterior=estimacion_inicial;
    nuevoProceso->duracionRafagaAnterior=0;
    nuevoProceso->estimadoSiguienteRafaga=0;

    // Intentar inicializar el proceso en memoria
    if (inicializar_proceso_memoria(nuevoProceso->PID, tam)) {
        if(algoritmoColaNewEnFIFO)
            agregarALista(listaProcesosNew, nuevoProceso);
        else
            agregarAListaOrdenada(listaProcesosNew, nuevoProceso, menorTam);

        temporal_resume(nuevoProceso->cronometros[NEW]);
        nuevoProceso->ME[NEW]++; 

        log_info(loggerKernel, "## (<%u>) Se crea el proceso - Estado: NEW", nuevoProceso->PID);
        
        if(nuevoProceso->PID == 0) { // Si es el primer proceso, espero el ENTER
            while (1) {
                char* input = readline("Apriete ENTER para empezar a planificar procesos.\n");  
                if (*input == '\0') {  
                    break;
                }
            }
        }    
        
        inicializarProceso();
    } else {
        log_error(loggerKernel, "## (<%u>) - Error al inicializar proceso en memoria", nuevoProceso->PID);
        // Liberar recursos del proceso que no pudo inicializarse
        for (int i = 0; i < 7; i++) {
            temporal_destroy(nuevoProceso->cronometros[i]);
        }
        free(nuevoProceso->archivoPseudocodigo);
        free(nuevoProceso);
    }
}

void dump_memory(uint32_t pid) {
    log_info(loggerKernel, "## (<%d>) - Solicitó syscall: DUMP_MEMORY", pid);

    PCB* proceso = buscarPCBEjecutando(pid);
    if (proceso == NULL) {
        log_error(loggerKernel, "## (<%u>) - No se encontró el PCB para DUMP_MEMORY", pid);
        exit(1);
    }

    if (solicitar_dump_memoria(pid)) {
        log_info(loggerKernel, "## (<%u>) - Finalizó correctamente DUMP_MEMORY", pid);
        pasarAReady(proceso);
    } else {
        log_error(loggerKernel, "## (<%u>) - Error en DUMP_MEMORY. Finalizando proceso", pid);
        pasarAExit(proceso);
    }
}

void syscall_IO(uint32_t pid, char* nombreIO, int64_t tiempo) {
    log_info(loggerKernel, "## (<%u>) - Solicitó syscall: IO", pid);

    PCB* proceso = NULL;
    proceso  = buscarPCBEjecutando(pid);

    if (proceso == NULL) {
        log_error(loggerKernel, "## (<%u>) - No se encontró el PCB para syscall IO", pid);
        exit(1);
    }
    
    terminarEjecucion(proceso);
   


    DispositivoIO* dispositivo = NULL;
    dispositivo = leerDeDiccionario(diccionarioDispositivosIO,nombreIO);

    if (dispositivo == NULL) {
        log_error(loggerKernel, "## (<%u>) - Dispositivo IO %s no encontrado. Finalizando proceso", pid, nombreIO);
        pasarAExit(proceso);
        return;
    }

    log_info(loggerKernel, "## (<%u>) - Bloqueado por IO: <%s>",pid,nombreIO);

    pasarABLoqueado(proceso, tiempo, nombreIO);
}

void syscallExit(uint32_t pid)
{
    PCB* proceso = NULL;
    proceso = buscarPCBEjecutando(pid);

    if (proceso == NULL) {
        log_error(loggerKernel, "## (<%u>) - No se encontró el PCB para syscall IO", pid);
        exit(1);
    }

    terminarEjecucion(proceso);

    pasarAExit(proceso);

}
PCB* buscarPCBEjecutando(uint32_t pid) {
    bool _mismoPID(nucleoCPU* nucleoEnEjecucion) {
        return nucleoEnEjecucion->procesoEnEjecucion->PID == pid;
    };

    nucleoCPU* nucleoCPU = leerDeListaSegunCondicion(listaCPUsEnUso,_mismoPID);
    if(nucleoCPU!= NULL)
        return nucleoCPU->procesoEnEjecucion;
    else
        return NULL;
}


