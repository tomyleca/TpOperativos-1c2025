#include "kernel.h"

void* planificadorCortoPlazo(void* arg)
{
    PCB* procesoAEjecutar;
    while(1)
    {
        sem_wait(semaforoIntentarPlanificar);
        
        switch(algoritmoDePlanificacionInt){
            
            
            


            case FIFO:
                procesoAEjecutar  = sacarDeLista(listaProcesosReady,0);
                break;
            case SJF:
                ordenarLista(listaProcesosReady,menorEstimadoRafagaActual);
                procesoAEjecutar = sacarDeLista(listaProcesosReady,0);
                break;
                
        case SRT: 
                if(!chequearListaVacia(listaCPUsLibres))
                {   
                    ordenarLista(listaProcesosReady,menorEstimadoRafagaActual);
                    procesoAEjecutar = sacarDeLista(listaProcesosReady,0);
                    break;
                }
                else 
                {
                    ordenarLista(listaProcesosReady,menorEstimadoRafagaActual);
                    procesoAEjecutar = sacarDeLista(listaProcesosReady,0);
                    if(chequearSiHayDesalojo(procesoAEjecutar->estimadoRafagaActual) == false)
                        procesoAEjecutar = NULL;
                    
                    break;
                }

                
                
        }


        if(procesoAEjecutar != NULL)
        {
            cargarCronometro(procesoAEjecutar,READY);
            cargarCronometro(procesoAEjecutar,SWAP_READY);
            ejecutar(procesoAEjecutar);
           
        }
    }

}


void ejecutar(PCB* proceso)
{
    nucleoCPU* nucleoCPUEnEjecucion = sacarDeLista(listaCPUsLibres,0);
    nucleoCPUEnEjecucion->ejecutando=true;
    nucleoCPUEnEjecucion->procesoEnEjecucion=proceso;
    agregarALista(listaCPUsEnUso,nucleoCPUEnEjecucion);

    proceso->ME[EXECUTE]++;
    temporal_resume(proceso->cronometros[EXECUTE]);
    proceso->cronometroEjecucionActual=temporal_create();

}

void guardarDatosDeEjecucion(PCB* procesoDespuesDeEjecucion)
{
     
    cargarCronometro(procesoDespuesDeEjecucion,EXECUTE);
    

    procesoDespuesDeEjecucion->duracionRafagaAnterior=temporal_gettime(procesoDespuesDeEjecucion->cronometroEjecucionActual);
    procesoDespuesDeEjecucion->estimadoRafagaAnterior=procesoDespuesDeEjecucion->estimadoRafagaActual;
    estimarRafagaActual(procesoDespuesDeEjecucion);

    temporal_destroy(procesoDespuesDeEjecucion->cronometroEjecucionActual);

}


 bool chequearSiHayDesalojo(int64_t estimadoRafagaProcesoEnEspera)
{
    
    bool _menorRafaga(nucleoCPU* CPU)
    {
        return estimadoRafagaProcesoEnEspera < temporal_gettime(CPU->procesoEnEjecucion->cronometroEjecucionActual);
    };
    
    sem_wait(listaCPUsEnUso->semaforoMutex);
    t_list* listaCPUsOrdenadaPorRafagaRestante = list_sorted(listaCPUsEnUso->lista,menorEstimadoRafagaRestante);
    sem_post(listaCPUsEnUso->semaforoMutex);
    nucleoCPU* nucleoConMenorRafagaRestante = NULL;
    

    nucleoConMenorRafagaRestante = list_remove_by_condition(listaCPUsOrdenadaPorRafagaRestante,_menorRafaga);
    
    
            
    if(nucleoConMenorRafagaRestante!=NULL)
    {
        
        PCB* procesoDesalojado = terminarEjecucionNucleoCPU(nucleoConMenorRafagaRestante);
        pasarAReady(procesoDesalojado);
        return true;
    }
    else
    {
        return false;
    }

    list_clean(listaCPUsOrdenadaPorRafagaRestante);
    free(listaCPUsOrdenadaPorRafagaRestante);


}

PCB* terminarEjecucionNucleoCPU(nucleoCPU* nucleoCPU)
{
    sacarElementoDeLista(listaCPUsEnUso,nucleoCPU);
    nucleoCPU->ejecutando=false;
    agregarALista(listaCPUsLibres,nucleoCPU);
    PCB* procesoPostEjecucion = nucleoCPU->procesoEnEjecucion;
    guardarDatosDeEjecucion(procesoPostEjecucion);
    return procesoPostEjecucion;
}

bool menorEstimadoRafagaRestante(nucleoCPU* CPU1,nucleoCPU* CPU2)
{
    return temporal_gettime(CPU1->procesoEnEjecucion->cronometroEjecucionActual) >= temporal_gettime(CPU2->procesoEnEjecucion->cronometroEjecucionActual);
}



bool menorEstimadoRafagaActual(PCB* PCB1,PCB* PCB2)
{
    return PCB1->estimadoRafagaActual >= PCB2->estimadoRafagaActual;
}

void estimarRafagaActual(PCB* proceso)
{
    proceso->estimadoRafagaActual= alfa * proceso->duracionRafagaAnterior + (1- alfa) * proceso->estimadoRafagaAnterior;
}

