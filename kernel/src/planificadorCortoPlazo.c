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
                    procesoAEjecutar = leerDeLista(listaProcesosReady,0); //Si la lista esta vacía se queda esperando
                    if(chequearSiHayDesalojo(procesoAEjecutar->estimadoSiguienteRafaga) == true && !chequearListaVacia(listaCPUsLibres))
                        procesoAEjecutar = sacarDeLista(listaProcesosReady,0);
                    else
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
    int64_t tiempoProcesoActualEnEjecucion;
    bool _menorRafagaQueProcesoEnReady(NucleoCPU* CPU)
    {
        tiempoProcesoActualEnEjecucion = temporal_gettime(CPU->procesoEnEjecucion->cronometroEjecucionActual);
        return estimadoRafagaProcesoEnEspera < tiempoProcesoActualEnEjecucion;
    };
    

    NucleoCPU* nucleoADesalojar = NULL;
    

    nucleoADesalojar = leerDeListaSegunCondicion(listaCPUsEnUso,_menorRafagaQueProcesoEnReady); //Si la rafaga del proceso en ready es menor  a la del cpu con menor rafaga restante devuelve ese cpu, sino devuelve NULL
    
    
            
    if(nucleoADesalojar!=NULL && nucleoADesalojar->procesoEnEjecucion != NULL)
    {
        
        if(terminarEjecucion(nucleoADesalojar->procesoEnEjecucion,INTERRUPCION_ASINCRONICA) == 1) //si es igual a 1 quiere decir que la ejecución termino por el desalojo y no por una syscall
            {
            PCB* procesoDesalojado = nucleoADesalojar->procesoEnEjecucion;
            log_info(loggerKernel, "## (<%u>) - Desalojado por algoritmo SJF/SRT",procesoDesalojado->PID);
            log_info(loggerKernel, "## (<%u>) Pasa del estado <%s> al estado <%s>",procesoDesalojado->PID,"EXECUTE","READY");
            pasarAReady(procesoDesalojado);
            return true;
            }
        else 
            return false;
    }
    else
    {
        return false;
    }




}

int terminarEjecucion(PCB* proceso,op_code tipoInterruccion)
{
    sem_wait(semaforoMutexTerminarEjecucion);
        bool _ejecutandoProceso(NucleoCPU* nucleoCPU)
        {
            return nucleoCPU->procesoEnEjecucion == proceso;
        };

        if(tipoInterruccion == INTERRUPCION_ASINCRONICA)
            agregarAListaSinRepetidos(listaProcesosPorSerDesalojados,proceso);
        
        
        NucleoCPU* nucleoCPU = sacarDeListaSegunCondicion(listaCPUsEnUso,_ejecutandoProceso);
        
        if(nucleoCPU != NULL)
            {
            nucleoCPU->ejecutando=false;
            mandarInterrupcion(nucleoCPU,tipoInterruccion);
            agregarALista(listaCPUsLibres,nucleoCPU);
            PCB* procesoPostEjecucion = nucleoCPU->procesoEnEjecucion;
            guardarDatosDeEjecucion(procesoPostEjecucion);

            sem_post(semaforoIntentarPlanificar);
            sem_post(semaforoMutexTerminarEjecucion);
            return 1;
            } /* Puede ocurrir que se intente finalizar la ejecucion de un proceso por desalojo al despues que el mismo se intenta 
            finalizar por algo que ocurre en su ejecución(SYSCALL EXIT, IO NO ENCONTRADO ETC.)
            , o visceversa, por lo que chequeo que haya un cpu ejecutando el proceso del que quiero terminar la ejecucion*/
            
            sem_post(semaforoMutexTerminarEjecucion);
            return 0;
        
}

bool menorEstimadoRafagaRestante(NucleoCPU* CPU1,NucleoCPU* CPU2)
{
    return temporal_gettime(CPU1->procesoEnEjecucion->cronometroEjecucionActual) <= temporal_gettime(CPU2->procesoEnEjecucion->cronometroEjecucionActual);
}



bool menorEstimadoSiguienteRafaga(PCB* PCB1,PCB* PCB2)
{
    return PCB1->estimadoSiguienteRafaga <= PCB2->estimadoSiguienteRafaga;
}

void estimarSiguienteRafaga(PCB* proceso)
{
    proceso->estimadoSiguienteRafaga= alfa * proceso->duracionRafagaAnterior + (1- alfa) * proceso->estimadoRafagaAnterior;

}