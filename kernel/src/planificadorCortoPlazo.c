#include "kernel.h"

void* planificadorCortoPlazo(void* arg)
{
    PCB* procesoAEjecutar= NULL;
    while(1)
    {
        sem_wait(semaforoIntentarPlanificar);
        
        switch(algoritmoDePlanificacionInt){
            
            
            


        case FIFO:
                procesoAEjecutar  = sacarDeLista(listaProcesosReady,0);
                break;
        case SJF:
                ordenarLista(listaProcesosReady,menorEstimadoSiguienteRafaga);
                procesoAEjecutar = sacarDeLista(listaProcesosReady,0);
                break;
                
        case SRT: 
                if(!chequearListaVacia(listaCPUsLibres))
                {   
                    ordenarLista(listaProcesosReady,menorEstimadoSiguienteRafaga);
                    procesoAEjecutar = sacarDeLista(listaProcesosReady,0);
                    break;
                }
                else 
                {
                    ordenarLista(listaProcesosReady,menorEstimadoSiguienteRafaga);
                    procesoAEjecutar = sacarDeLista(listaProcesosReady,0);
                    if(chequearSiHayDesalojo(procesoAEjecutar->estimadoSiguienteRafaga) == false)
                        procesoAEjecutar = NULL;
                    
                    break;
                }

                
                
        }


        if(procesoAEjecutar != NULL)
        {
            log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAEjecutar->PID,"READY","EXECUTE");
            cargarCronometro(procesoAEjecutar,READY);
            pasarAExecute(procesoAEjecutar);
           
        }
    }

}

void pasarAExecute(PCB* proceso)
{
    nucleoCPU* nucleoCPULibre =  sacarDeLista(listaCPUsLibres,0);
    nucleoCPULibre->ejecutando=true;
    nucleoCPULibre->procesoEnEjecucion=proceso;
    agregarALista(listaCPUsEnUso,nucleoCPULibre);
    
    mandarContextoACPU(proceso->PID,proceso->PC,nucleoCPULibre->fdConexion);

    proceso->cronometroEjecucionActual = temporal_create();
    temporal_resume(proceso->cronometros[EXECUTE]);
    proceso->ME[EXECUTE]++;
   
}



void guardarDatosDeEjecucion(PCB* procesoDespuesDeEjecucion)
{
     
    cargarCronometro(procesoDespuesDeEjecucion,EXECUTE);
    

    procesoDespuesDeEjecucion->duracionRafagaAnterior=temporal_gettime(procesoDespuesDeEjecucion->cronometroEjecucionActual);
    procesoDespuesDeEjecucion->estimadoRafagaAnterior=procesoDespuesDeEjecucion->estimadoSiguienteRafaga;
    estimarSiguienteRafaga(procesoDespuesDeEjecucion);

    temporal_stop(procesoDespuesDeEjecucion->cronometroEjecucionActual);
    
    //TODO arreglar esto con valgrind
    //temporal_destroy(procesoDespuesDeEjecucion->cronometroEjecucionActual);

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
        
        terminarEjecucion(nucleoConMenorRafagaRestante->procesoEnEjecucion);
        PCB* procesoDesalojado = nucleoConMenorRafagaRestante->procesoEnEjecucion;
        log_info(loggerKernel, "## (<%u>) - Desalojado por algoritmo SJF/SRT",procesoDesalojado->PID);
        log_info(loggerKernel, "## (<%u>) Pasa del estado <%s> al estado <%s>",procesoDesalojado->PID,"EXECUTE","READY");
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

void terminarEjecucion(PCB* proceso)
{
    bool _ejecutandoProceso(nucleoCPU* nucleoCPU)
    {
        return nucleoCPU->procesoEnEjecucion == proceso;
    };

    nucleoCPU* nucleoCPU = sacarDeListaSegunCondicion(listaCPUsEnUso,_ejecutandoProceso);
    nucleoCPU->ejecutando=false;
    agregarALista(listaCPUsLibres,nucleoCPU);
    PCB* procesoPostEjecucion = nucleoCPU->procesoEnEjecucion;
    guardarDatosDeEjecucion(procesoPostEjecucion);
    sem_post(semaforoIntentarPlanificar);
    
}

bool menorEstimadoRafagaRestante(nucleoCPU* CPU1,nucleoCPU* CPU2)
{
    return temporal_gettime(CPU1->procesoEnEjecucion->cronometroEjecucionActual) >= temporal_gettime(CPU2->procesoEnEjecucion->cronometroEjecucionActual);
}



bool menorEstimadoSiguienteRafaga(PCB* PCB1,PCB* PCB2)
{
    return PCB1->estimadoSiguienteRafaga >= PCB2->estimadoSiguienteRafaga;
}

void estimarSiguienteRafaga(PCB* proceso)
{
    proceso->estimadoSiguienteRafaga= alfa * proceso->duracionRafagaAnterior + (1- alfa) * proceso->estimadoRafagaAnterior;
}