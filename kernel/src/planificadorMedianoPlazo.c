#include "kernel.h"

void pasarABLoqueado(PCB* proceso,int64_t tiempo,char* nombreIO){
    
    log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",proceso->PID,"READY","BLOCKED");
    cargarCronometro(proceso,READY);
    


    
   
    
    procesoEnEsperaIO* procesoEsperando=malloc(sizeof(procesoEnEsperaIO));
    procesoEsperando->proceso=proceso;
    procesoEsperando->estaENSwap=0;
    procesoEsperando->tiempo=tiempo;
    procesoEsperando->semaforoIOFinalizada=malloc(sizeof(sem_t));
    procesoEsperando->semaforoMutex=malloc(sizeof(sem_t));
    sem_init(procesoEsperando->semaforoIOFinalizada,1,0);
    sem_init(procesoEsperando->semaforoMutex,1,1);

    char* PIDComoChar = pasarUnsignedAChar(proceso->PID);
    agregarADiccionario(diccionarioProcesosBloqueados,PIDComoChar,procesoEsperando);
    


    avisarInicioIO(procesoEsperando,nombreIO,tiempo);
    
    pthread_t hiloManejoBloqueado;
    pthread_create(&hiloManejoBloqueado,NULL,(void *)manejarProcesoBloqueado,procesoEsperando);
    

    
    

    free(PIDComoChar);

}


void* manejarProcesoBloqueado(procesoEnEsperaIO* procesoEnEsperaIO){
    

    
    char* PID = pasarUnsignedAChar(procesoEnEsperaIO->proceso->PID);
    t_temporal* cronometroBloqueadoActual = temporal_create();
    temporal_resume(cronometroBloqueadoActual);

    temporal_resume(procesoEnEsperaIO->proceso->cronometros[BLOCKED]);
    procesoEnEsperaIO->proceso->ME[BLOCKED]++;

    pthread_t hiloContadorSwap;
    pthread_create(&hiloContadorSwap,NULL,(void *)contadorParaSwap,procesoEnEsperaIO);

    sem_wait(procesoEnEsperaIO->semaforoIOFinalizada);
    
    
    
    sacarDeDiccionario(diccionarioProcesosBloqueados,PID);  //Desbloqueo el proceso
    
    sem_wait(procesoEnEsperaIO->semaforoMutex); //Mutex para chequear que el otro hilo no este en medio de un proceso
    esperarCancelacionDeHilo(hiloContadorSwap); //Cancelo el hilo contadorSwap, para que no tire seg fault cuando haga free del semaforoMutex
    
    if(procesoEnEsperaIO->estaENSwap == 0) //Chequeo que no se haya pasado a swap
    {

        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoEnEsperaIO->proceso->PID,"BLOCKED","READY");
        cargarCronometro(procesoEnEsperaIO->proceso,BLOCKED);
        pasarAReady(procesoEnEsperaIO->proceso);
        
        
        free(procesoEnEsperaIO->semaforoIOFinalizada);
        free(procesoEnEsperaIO->semaforoMutex);
        free(procesoEnEsperaIO);
        
    }
    else if(procesoEnEsperaIO->estaENSwap == 1)
    {
        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoEnEsperaIO->proceso->PID,"SWAP_BLOCKED","SWAP_READY");
        cargarCronometro(procesoEnEsperaIO->proceso,SWAP_BLOCKED);
        pasarASwapReady(procesoEnEsperaIO->proceso);
    }
    else
    {
        log_info(loggerKernel,"## (<%u>) ERROR. PROCESO EN ESTADO INCONSISTENTE",procesoEnEsperaIO->proceso->PID);
        exit(1);
    }
    
    //No hago post, el otro hilo ya lo cancele
    

    //TODO temporal destroy

    
    

    return NULL;
}

void esperarCancelacionDeHilo(pthread_t hiloACancelar)
{
    pthread_cancel(hiloACancelar);
    pthread_join(hiloACancelar,NULL);
}

void contadorParaSwap (procesoEnEsperaIO* procesoEnEsperaIO)
{
    
    usleep(tiempo_suspension*1000); //  *1000 para pasar de milisegundos a microsegundos //TODO ver si hay que pasarlo a microsegundos o como es
    
    //Paso el proceso a Swap
    sem_wait(procesoEnEsperaIO->semaforoMutex); 
    pasarASwapBlocked(procesoEnEsperaIO);
    log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoEnEsperaIO->proceso->PID,"BLOCKED","SWAP_BLOCKED");
    cargarCronometro(procesoEnEsperaIO->proceso,BLOCKED);
    sem_post(procesoEnEsperaIO->semaforoMutex);
      

                
       
}



void pasarASwapBlocked(procesoEnEsperaIO* procesoEsperandoIO)
{
    //TODO Le aviso a la memoria que el proceso paso a disco.
    
    temporal_resume(procesoEsperandoIO->proceso->cronometros[SWAP_BLOCKED]);
    procesoEsperandoIO->proceso->ME[SWAP_BLOCKED]++;
    procesoEsperandoIO->estaENSwap=1;


    inicializarProceso();
}


void pasarASwapReady(PCB* proceso)
{
    temporal_resume(proceso->cronometros[SWAP_READY]);
    proceso->MT[SWAP_READY]++;

    if(algoritmoColaNewEnFIFO)
    {
        agregarALista(listaProcesosSwapReady,proceso);
    }
    else 
    {
        agregarAListaOrdenada(listaProcesosSwapReady,proceso,menorTam);
    }


    inicializarProceso();
}

void manejarFinDeIO(uint32_t PID,char* nombreDispositivoIO)
{
    DispositivoIO* dispositivoIOLiberado = leerDeDiccionario(diccionarioDispositivosIO,nombreDispositivoIO);
    sem_post(dispositivoIOLiberado->semaforoDispositivoOcupado);
    
    if(!list_is_empty(dispositivoIOLiberado->colaEsperandoIO->lista)) //Si la cola de procesos en espera no esta vacía, empiezo el IO del proceso esperando
        empezarIODelProximoEnEspera(dispositivoIOLiberado);
    
    char* PIDComoChar = pasarUnsignedAChar(PID);
    procesoEnEsperaIO* procesoADesbloquear = leerDeDiccionario(diccionarioProcesosBloqueados,PIDComoChar);
    sem_post(procesoADesbloquear->semaforoIOFinalizada);

    log_info(loggerKernel, "## (<%u>) finalizó IO y pasa a READY",PID);

    
}

//TODO probarlo
void empezarIODelProximoEnEspera(DispositivoIO* dispositivoIO)
{
    procesoEnEsperaIO* procesoEnEsperaIO = sacarDeLista(dispositivoIO->colaEsperandoIO,0);
    avisarInicioIO(procesoEnEsperaIO,dispositivoIO->nombre,procesoEnEsperaIO->tiempo);
}