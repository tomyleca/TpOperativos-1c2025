#include "kernel.h"

void* planificadorCortoPlazo(void* arg)
{
    PCB* procesoAEjecutar;
    while(1)
    {
    switch(algoritmoDePlanificacionInt){
        
        
        sem_wait(semaforoIntentarPlanificar);


        case FIFO:
             procesoAEjecutar  = sacarDeLista(listaProcesosReady,0);
            break;
        case SJF:
            ordenarLista(listaProcesosReady,menorEstimadoRafagaActual);
            procesoAEjecutar = sacarDeLista(listaProcesosReady,0);
            break;
            
       case SRT: 
            ordenarLista(listaProcesosReady,menorEstimadoRafagaActual);
            procesoAEjecutar = sacarDeLista(listaProcesosReady,0);
            
            break;
    }
    
    pthread_t* hiloEjecucion = malloc(sizeof(pthread_t));
    pthread_create(hiloEjecucion,NULL,ejecutar,procesoAEjecutar);
    pthread_detach(*hiloEjecucion);

    }
}

void* ejecutar(PCB* proceso)
{
    nucleoCPU* nucleoCPUEnEjecucion = sacarDeLista(listaCPUsLibres,0);
    agregarALista(listaCPUsEnUso,nucleoCPUEnEjecucion);
    nucleoCPUEnEjecucion->ejecutando=true;
    nucleoCPUEnEjecucion->procesoEnEjecucion=proceso;

    proceso->ME[EXECUTE]++;

    t_temporal* cronometroTiempoEnEjecucion=temporal_create();
    while(nucleoCPUEnEjecucion->ejecutando)
    {
        nucleoCPUEnEjecucion->estimadoRafagaRestante=proceso->estimadoRafagaActual - temporal_gettime(cronometroTiempoEnEjecucion); //Le resto lo ya transcurrido para estimar cuanto le queda
    }
    
    temporal_stop(cronometroTiempoEnEjecucion);

    PCB* procesoDespuesDeEjecucion = proceso;
    
    int64_t tiempoEnEjecucion = temporal_gettime(cronometroTiempoEnEjecucion);
    procesoDespuesDeEjecucion->MT[EXECUTE]+=tiempoEnEjecucion;


    procesoDespuesDeEjecucion->duracionRafagaAnterior=tiempoEnEjecucion;
    procesoDespuesDeEjecucion->estimadoRafagaAnterior=procesoDespuesDeEjecucion->estimadoRafagaActual;
    estimarRafagaActual(procesoDespuesDeEjecucion);

    free(cronometroTiempoEnEjecucion);
    
}

 bool chequearSiHayDesalojo(int64_t estimadoRafagaProcesoEnEspera)
{
    sem_wait(listaCPUsEnUso->semaforoMutex);
    bool _menorRafaga(nucleoCPU* CPU)
    {
        return estimadoRafagaProcesoEnEspera < CPU->estimadoRafagaRestante;
    };
    
    t_list* listaCPUsOrdenadaPorRafagaRestante = list_sorted(listaCPUsEnUso->lista,menorEstimadoRafagaRestante);
    nucleoCPU* nucleoConMenorRafagaRestante = NULL;
    nucleoConMenorRafagaRestante = list_remove_by_condition(listaCPUsOrdenadaPorRafagaRestante,_menorRafaga);
    if(nucleoConMenorRafagaRestante!=NULL)
    {
        sem_wait(listaCPUsEnUso->semaforoCantElementos);
        agregarALista(listaCPUsLibres,nucleoConMenorRafagaRestante);
        return true;
    }
    else
    {
        return false;
    }

    sem_post(listaCPUsEnUso->semaforoMutex);
            
}

bool menorEstimadoRafagaRestante(nucleoCPU* CPU1,nucleoCPU* CPU2)
{
    return CPU1->estimadoRafagaRestante >= CPU2->estimadoRafagaRestante;
}



bool menorEstimadoRafagaActual(PCB* PCB1,PCB* PCB2)
{
    return PCB1->estimadoRafagaActual >= PCB2->estimadoRafagaActual;
}

void estimarRafagaActual(PCB* proceso)
{
    proceso->estimadoRafagaActual= alfa * proceso->duracionRafagaAnterior + (1- alfa) * proceso->estimadoRafagaAnterior;
}

