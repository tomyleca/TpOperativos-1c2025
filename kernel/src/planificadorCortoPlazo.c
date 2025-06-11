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
                procesoAEjecutar = sacarDeLista(listaProcesosReady,0); //Si la lista esta vacía se queda esperando
                break;
                
        case SRT: 
                if(!chequearListaVacia(listaCPUsLibres))
                {   
                    ordenarLista(listaProcesosReady,menorEstimadoSiguienteRafaga);
                    procesoAEjecutar = sacarDeLista(listaProcesosReady,0); //Si la lista esta vacía se queda esperando
                    break;
                }
                else 
                {
                    ordenarLista(listaProcesosReady,menorEstimadoSiguienteRafaga);
                    procesoAEjecutar = sacarDeLista(listaProcesosReady,0); //Si la lista esta vacía se queda esperando
                    if(chequearSiHayDesalojo(procesoAEjecutar->estimadoSiguienteRafaga) == false)
                        procesoAEjecutar = NULL;
                    
                    break;
                }

                
                
        }


        if(procesoAEjecutar != NULL && procesoAEjecutar->PID >= 0) //Lo del PID es un chequeo de que estoy apuntando a un struct de tipo PCB
        {
            log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAEjecutar->PID,"READY","EXECUTE");
            cargarCronometro(procesoAEjecutar,READY);
            pasarAExecute(procesoAEjecutar);
           
        }
    }

}

void pasarAExecute(PCB* proceso)
{
    NucleoCPU* nucleoCPULibre =  sacarDeLista(listaCPUsLibres,0);
    nucleoCPULibre->ejecutando=true;
    nucleoCPULibre->procesoEnEjecucion=proceso;
    agregarALista(listaCPUsEnUso,nucleoCPULibre);
    
    proceso->cronometroEjecucionActual = temporal_create();
    temporal_resume(proceso->cronometros[EXECUTE]);
    proceso->ME[EXECUTE]++;
    mandarContextoACPU(proceso->PID,proceso->PC,nucleoCPULibre->fdConexionDispatch);


   
}



void guardarDatosDeEjecucion(PCB* procesoDespuesDeEjecucion)
{
     
    cargarCronometro(procesoDespuesDeEjecucion,EXECUTE);
    
    temporal_stop(procesoDespuesDeEjecucion->cronometroEjecucionActual);
    procesoDespuesDeEjecucion->duracionRafagaAnterior=temporal_gettime(procesoDespuesDeEjecucion->cronometroEjecucionActual);
    procesoDespuesDeEjecucion->estimadoRafagaAnterior=procesoDespuesDeEjecucion->estimadoSiguienteRafaga;
    estimarSiguienteRafaga(procesoDespuesDeEjecucion);

    
    
    temporal_destroy(procesoDespuesDeEjecucion->cronometroEjecucionActual);

}


 bool chequearSiHayDesalojo(int64_t estimadoRafagaProcesoEnEspera)
{
    
    bool _menorRafaga(NucleoCPU* CPU)
    {
        return estimadoRafagaProcesoEnEspera < temporal_gettime(CPU->procesoEnEjecucion->cronometroEjecucionActual);
    };
    
    sem_wait(listaCPUsEnUso->semaforoMutex);
        t_list* listaCPUsOrdenadaPorRafagaRestante = list_sorted(listaCPUsEnUso->lista,menorEstimadoRafagaRestante);
    sem_post(listaCPUsEnUso->semaforoMutex);
    NucleoCPU* nucleoConMenorRafagaRestante = NULL;
    

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
    bool _ejecutandoProceso(NucleoCPU* NucleoCPU)
    {
        return NucleoCPU->procesoEnEjecucion == proceso;
    };

    NucleoCPU* NucleoCPU = sacarDeListaSegunCondicion(listaCPUsEnUso,_ejecutandoProceso);
    NucleoCPU->ejecutando=false;
    mandarInterrupcion(NucleoCPU);
    agregarALista(listaCPUsLibres,NucleoCPU);
    PCB* procesoPostEjecucion = NucleoCPU->procesoEnEjecucion;
    guardarDatosDeEjecucion(procesoPostEjecucion);
    
    sem_post(semaforoIntentarPlanificar);
    
    
}

bool menorEstimadoRafagaRestante(NucleoCPU* CPU1,NucleoCPU* CPU2)
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