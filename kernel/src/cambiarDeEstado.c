#include "cambiarDeEstado.h"
#include "conexionConMemoria.h"


void pasarASwapBlocked(procesoEnEsperaIO* procesoEsperandoIO) {
    PCB* proceso = procesoEsperandoIO->proceso;
    
    if (suspender_proceso_memoria(proceso->PID)) {
        temporal_stop(proceso->cronometros[BLOCKED]);
        temporal_resume(proceso->cronometros[SUSP_BLOCKED]);
        proceso->ME[BLOCKED]--;
        proceso->ME[SUSP_BLOCKED]++;
        
        log_info(loggerKernel, "## (<%u>) Pasa del estado BLOCKED al estado SUSP. BLOCKED", proceso->PID);
        
        removerDeLista(listaProcesosBloqueados, procesoEsperandoIO);
        agregarALista(listaProcesosSuspendidosBloqueados, procesoEsperandoIO);
    } else {
        log_error(loggerKernel, "## (<%u>) - Error al suspender proceso en memoria", proceso->PID);
    }
}

void pasarASwapReady(PCB* proceso) {
    if (des_suspender_proceso_memoria(proceso->PID)) {
        temporal_stop(proceso->cronometros[SUSP_READY]);
        temporal_resume(proceso->cronometros[READY]);
        proceso->ME[SUSP_READY]--;
        proceso->ME[READY]++;
        
        log_info(loggerKernel, "## (<%u>) Pasa del estado SUSP. READY al estado READY", proceso->PID);
        
        removerDeLista(listaProcesosSuspendidosReady, proceso);
        agregarALista(listaProcesosReady, proceso);
    } else {
        log_error(loggerKernel, "## (<%u>) - Error al des-suspender proceso en memoria", proceso->PID);
    }
}

void pasarAExit(PCB* proceso) {
    if (finalizar_proceso_memoria(proceso->PID)) {
        temporal_stop(proceso->cronometros[proceso->estadoActual]);
        proceso->ME[proceso->estadoActual]--;
        proceso->ME[EXIT]++;
        temporal_resume(proceso->cronometros[EXIT]);
        
        log_info(loggerKernel, "## (<%u>) Pasa del estado %s al estado EXIT", 
                proceso->PID, estadoAString(proceso->estadoActual));
        
        switch(proceso->estadoActual) {
            case READY:
                removerDeLista(listaProcesosReady, proceso);
                break;
            case BLOCKED:
                procesoEnEsperaIO* procesoEnEspera = buscarProcesoEnEsperaIO(proceso->PID);
                if (procesoEnEspera != NULL) {
                    removerDeLista(listaProcesosBloqueados, procesoEnEspera);
                }
                break;
            case SUSP_READY:
                removerDeLista(listaProcesosSuspendidosReady, proceso);
                break;
            case SUSP_BLOCKED:
                procesoEnEsperaIO* procesoSuspendido = buscarProcesoSuspendidoEnEsperaIO(proceso->PID);
                if (procesoSuspendido != NULL) {
                    removerDeLista(listaProcesosSuspendidosBloqueados, procesoSuspendido);
                }
                break;
            case EXEC:
                nucleoCPU* nucleo = buscarNucleoCPU(proceso->PID);
                if (nucleo != NULL) {
                    nucleo->procesoEnEjecucion = NULL;
                    nucleo->ocupado = false;
                }
                break;
            default:
                break;
        }
        
        for (int i = 0; i < 7; i++) {
            temporal_destroy(proceso->cronometros[i]);
        }
        free(proceso->archivoPseudocodigo);
        free(proceso);
        
        log_info(loggerKernel, "## (<%u>) - Finaliza el proceso", proceso->PID);
    } else {
        log_error(loggerKernel, "## (<%u>) - Error al finalizar proceso en memoria", proceso->PID);
    }
}

