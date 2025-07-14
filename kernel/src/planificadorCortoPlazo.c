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
                ordenarLista(listaProcesosReady,menorEstimadoSiguienteRafaga);
                procesoAEjecutar = sacarDeLista(listaProcesosReady,0); //Si la lista esta vacía se queda esperando
                break;
                
            }

       
        

        if(procesoAEjecutar != NULL && procesoAEjecutar->PID >= 0) //Lo del PID es un chequeo de que estoy apuntando a un struct de tipo PCB
        {
            
            cargarCronometro(procesoAEjecutar,READY);
            pasarAExecute(procesoAEjecutar);
            log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAEjecutar->PID,"READY","EXECUTE");
            procesoAEjecutar = NULL;
            
           
        }
    }

}

void pasarAExecute(PCB* proceso)
{
   
    NucleoCPU* nucleoCPULibre =  sacarDeLista(listaCPUsLibres,0);
    nucleoCPULibre->ejecutando=true;
    nucleoCPULibre->procesoEnEjecucion=proceso;
    agregarALista(listaCPUsEnUso,nucleoCPULibre);
    mandarContextoACPU(proceso->PID,proceso->PC,nucleoCPULibre->fdConexionDispatch);
    //proceso->cronometroEjecucionActual = temporal_create();
    temporal_resume(proceso->cronometros[EXECUTE]);
    proceso->ME[EXECUTE]++;
    


   
}



void guardarDatosDeEjecucion(PCB* procesoDespuesDeEjecucion)
{

    procesoDespuesDeEjecucion->duracionRafagaAnterior=temporal_gettime(procesoDespuesDeEjecucion->cronometros[EXECUTE]) - procesoDespuesDeEjecucion->MT[EXECUTE]; //Saco cuanto es lo ultimo que ejecuto
    cargarCronometro(procesoDespuesDeEjecucion,EXECUTE);
    procesoDespuesDeEjecucion->estimadoRafagaAnterior=procesoDespuesDeEjecucion->estimadoSiguienteRafaga;
    estimarSiguienteRafaga(procesoDespuesDeEjecucion);

    
    

    

}


 bool chequearSiHayDesalojo(int64_t estimadoRafagaProcesoEnEspera)
{
    int64_t tiempoRestanteProcesoActualEnEjecucion;
    bool _menorRafagaQueProcesoEnReady(void* arg)
    {
        NucleoCPU* CPU = (NucleoCPU*) arg;                                                            //Lo que lleva ejecutado                                                //Lo que ejecuto antes de la ejecución            
        tiempoRestanteProcesoActualEnEjecucion = CPU->procesoEnEjecucion->estimadoSiguienteRafaga - (temporal_gettime(CPU->procesoEnEjecucion->cronometros[EXECUTE]) - CPU->procesoEnEjecucion->MT[EXECUTE]);
        return estimadoRafagaProcesoEnEspera < tiempoRestanteProcesoActualEnEjecucion;
    };
    

    NucleoCPU* nucleoADesalojar = NULL;
    
    sem_wait(semaforoMutexTerminarEjecucion);
        nucleoADesalojar = leerDeListaSegunCondicion(listaCPUsEnUso,_menorRafagaQueProcesoEnReady); //Si la rafaga del proceso en ready es menor  a la del cpu con menor rafaga restante devuelve ese cpu, sino devuelve NULL
    sem_post(semaforoMutexTerminarEjecucion);
    
            
    if(nucleoADesalojar!=NULL && nucleoADesalojar->procesoEnEjecucion != NULL)
    {
        
        if(terminarEjecucion(nucleoADesalojar->procesoEnEjecucion->PID,INTERRUPCION_ASINCRONICA) != NULL) //si no es igual a NULL quiere decir que la ejecución termino por el desalojo y no por una syscall
            {
            desalojarProceso(nucleoADesalojar,nucleoADesalojar->procesoEnEjecucion);
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

void desalojarProceso(NucleoCPU* nucleoADesalojar,PCB* proceso)
{   
    sem_wait(semaforoMutexExit);
    
        if(proceso == NULL) //si da false es pq el proceso se paso a exit
            return;

        agregarAListaSinRepetidos(listaProcesosPorSerDesalojados,proceso);
        log_debug(loggerKernel,"Se va a desalojar el proceso %u",nucleoADesalojar->procesoEnEjecucion->PID);
    sem_post(semaforoMutexExit);
    
    enviarOpCode(nucleoADesalojar->fdConexionInterrupt,INTERRUPCION_ASINCRONICA); //mando la interrupcion
    
    sem_wait(semaforoPCActualizado);      
    sem_wait(semaforoEnCheckInterrupt);                               
    
    if(sacarElementoDeLista(listaCPUsEnUso,nucleoADesalojar)!=false) // recien ahora lo puedo sacar para que no cause problemas
        agregarAListaSinRepetidos(listaCPUsLibres,nucleoADesalojar);

    sem_wait(semaforoMutexExit);
        log_info(loggerKernel, "## (<%u>) - Desalojado por algoritmo SJF/SRT",proceso->PID);
        log_info(loggerKernel, "## (<%u>) Pasa del estado <%s> al estado <%s>",proceso->PID,"EXECUTE","READY");
        pasarAReady(proceso,true);
    sem_post(semaforoMutexExit);

    
}


PCB* terminarEjecucion(uint32_t PID,op_code tipoInterruccion)
{
    NucleoCPU* nucleoCPU;
    
    sem_wait(semaforoMutexTerminarEjecucion);
        bool _ejecutandoProceso(void* arg)
        {
            NucleoCPU* nucleoCPU = (NucleoCPU*) arg;
            return nucleoCPU->procesoEnEjecucion->PID == PID;
        };

        PCB* procesoPostEjecucion = NULL;

        if(tipoInterruccion == INTERRUPCION_SINCRONICA)
        {
            nucleoCPU = sacarDeListaSegunCondicion(listaCPUsEnUso,_ejecutandoProceso);
            procesoPostEjecucion = nucleoCPU->procesoEnEjecucion; //como en sincronica no me interesa que retorne si lo saco de ejecucion ahora o antes puedo agregarlo aca
        }
        else 
            nucleoCPU = leerDeListaSegunCondicion(listaCPUsEnUso,_ejecutandoProceso); //para asincronica lo saco despues para que no haya problemas si esta
                                                                                        // a mitad de una syscall
        

        if(nucleoCPU->ejecutando) //osea que esta el proceso no habia sido sacado de ejecucion antes
        {
            nucleoCPU->ejecutando=false;
            procesoPostEjecucion = nucleoCPU->procesoEnEjecucion;
            guardarDatosDeEjecucion(procesoPostEjecucion);
            
            if(tipoInterruccion== INTERRUPCION_SINCRONICA)  
                agregarAListaSinRepetidos(listaCPUsLibres,nucleoCPU);
            
            sem_post(semaforoIntentarPlanificar);
            log_debug(loggerKernel,"(<%u>) termina su ejecución",procesoPostEjecucion->PID);
        }
            

        
    sem_post(semaforoMutexTerminarEjecucion);
            
            
    return procesoPostEjecucion;
} 
        
  
           
        





bool menorEstimadoSiguienteRafaga(void* PCB1,void* PCB2)
{
    PCB* proceso1 = (PCB*) PCB1;
    PCB* proceso2 = (PCB*) PCB2;
    log_debug(loggerKernel,"<%u> ESTIMADO SIGUIENTE RAFAGA: %d", proceso1->PID,(int)proceso1->estimadoSiguienteRafaga);
    log_debug(loggerKernel,"<%u> ESTIMADO SIGUIENTE RAFAGA: %d", proceso2->PID,(int)proceso2->estimadoSiguienteRafaga);
    return proceso1->estimadoSiguienteRafaga <= proceso2->estimadoSiguienteRafaga;
}

void estimarSiguienteRafaga(PCB* proceso)
{
     proceso->estimadoSiguienteRafaga= alfa * proceso->duracionRafagaAnterior + (1- alfa) * proceso->estimadoRafagaAnterior;

}