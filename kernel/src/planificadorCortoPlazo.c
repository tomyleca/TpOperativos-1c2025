#include "kernel.h"

void planificadorCortoPlazo()
{
    switch(algoritmoDePlanificacionInt){
        PCB* procesoAEjecutar;
        

        case FIFO:
            procesoAEjecutar  = sacarDeLista(listaProcesosReady,0);
            break;
        case SJF:
        case SRT:
            ordenarLista( listaProcesosReady,menorEstimadoRafagaActual);
            procesoAEjecutar = sacarDeLista(listaProcesosReady,0);
            break;
        

        
        
    
    
    t_temporal* cronometroTiempoEnEjecucion=temporal_create();
    procesoAEjecutar->ME[EXECUTE]++;

    //Espero que se me avise de la finalización del proceso 
    temporal_stop(cronometroTiempoEnEjecucion);

    PCB* procesoDespuesDeEjecucion = procesoAEjecutar;
    
    int64_t tiempoEnEjecucion = temporal_gettime(cronometroTiempoEnEjecucion);
    procesoDespuesDeEjecucion->MT[EXECUTE]+=tiempoEnEjecucion;


    procesoDespuesDeEjecucion->duracionRafagaAnterior=tiempoEnEjecucion;
    procesoDespuesDeEjecucion->estimadoRafagaAnterior=procesoDespuesDeEjecucion->estimadoRafagaActual;
    estimarRafagaActual(procesoDespuesDeEjecucion);

    free(cronometroTiempoEnEjecucion);
    
    }
}

bool menorEstimadoRafagaActual(PCB* PCB1,PCB* PCB2)
{
    return PCB1->estimadoRafagaActual <= PCB2->estimadoRafagaActual;
}

void estimarRafagaActual(PCB* proceso)
{
    proceso->estimadoRafagaActual= alfa * proceso->duracionRafagaAnterior + (1- alfa) * proceso->estimadoRafagaAnterior;
}

