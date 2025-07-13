#include "kernel.h"

void pasarABLoqueadoPorIO(PCB* proceso,int64_t tiempo,char* nombreIO){
    
    ProcesoEnEsperaIO* procesoEsperando=malloc(sizeof(ProcesoEnEsperaIO));
    procesoEsperando->proceso=proceso;
    procesoEsperando->estaENSwap=0;
    procesoEsperando->tiempo=tiempo;
    procesoEsperando->semaforoIOFinalizada=malloc(sizeof(sem_t));
    procesoEsperando->semaforoMutex=malloc(sizeof(sem_t));
    sem_init(procesoEsperando->semaforoIOFinalizada,1,0);
    sem_init(procesoEsperando->semaforoMutex,1,1);

    char* clave=pasarUnsignedAChar(proceso->PID);
    agregarADiccionario(diccionarioProcesosBloqueados,clave,procesoEsperando);
    free(clave);
    


    
    if(avisarInicioIO(procesoEsperando,nombreIO,tiempo) != -1)
    {
        log_info(loggerKernel, "## (<%u>) - Bloqueado por IO: <%s>",procesoEsperando->proceso->PID,nombreIO);
        pthread_t hiloContadorSwap;
        pthread_create(&hiloContadorSwap,NULL,(void *)contadorParaSwap,procesoEsperando);
        pthread_detach(hiloContadorSwap);
        procesoEsperando->hiloContadorSwap = hiloContadorSwap;
        pthread_t hiloManejoBloqueado;
        pthread_create(&hiloManejoBloqueado,NULL,(void *)manejarProcesoBloqueadoPorIO,procesoEsperando);
        pthread_detach(hiloManejoBloqueado);
        procesoEsperando->hiloManejoBloqueado= hiloManejoBloqueado;


        

        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",proceso->PID,"EXECUTE","BLOCKED");
        
   
    }
    else
    {
        
        log_error(loggerKernel, "## (<%u>) - Dispositivo IO %s no encontrado. Finalizando proceso", procesoEsperando->proceso->PID, nombreIO);
        pasarAExit(procesoEsperando->proceso,"EXECUTE");
        free(procesoEsperando->semaforoMutex);
        free(procesoEsperando->semaforoIOFinalizada);
        free(procesoEsperando);
    }
    

    
}


void* manejarProcesoBloqueadoPorIO(ProcesoEnEsperaIO* ProcesoEnEsperaIO){
    
    char* PID = pasarUnsignedAChar(ProcesoEnEsperaIO->proceso->PID);
 
    temporal_resume(ProcesoEnEsperaIO->proceso->cronometros[BLOCKED]);
    ProcesoEnEsperaIO->proceso->ME[BLOCKED]++;




    sem_wait(ProcesoEnEsperaIO->semaforoIOFinalizada);
    
    sacarDeDiccionario(diccionarioProcesosBloqueados,PID);  //Desbloqueo el proceso

    free(PID);
    
    sem_wait(ProcesoEnEsperaIO->semaforoMutex); //Mutex para chequear que el otro hilo no este en medio de un proceso
    esperarCancelacionDeHilo(ProcesoEnEsperaIO->hiloContadorSwap); //Cancelo el hilo contadorSwap, para que no tire seg fault cuando haga free del semaforoMutex
    
    if(ProcesoEnEsperaIO->estaENSwap == 0) //Chequeo que no se haya pasado a swap
    {

        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",ProcesoEnEsperaIO->proceso->PID,"BLOCKED","READY");
        cargarCronometro(ProcesoEnEsperaIO->proceso,BLOCKED);
        pasarAReady(ProcesoEnEsperaIO->proceso,false);
        
        
        free(ProcesoEnEsperaIO->semaforoIOFinalizada);
        free(ProcesoEnEsperaIO->semaforoMutex);
        free(ProcesoEnEsperaIO);
        
    }
    else if(ProcesoEnEsperaIO->estaENSwap == 1)
    {
        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",ProcesoEnEsperaIO->proceso->PID,"SWAP_BLOCKED","SWAP_READY");
        cargarCronometro(ProcesoEnEsperaIO->proceso,SWAP_BLOCKED);
        pasarASwapReady(ProcesoEnEsperaIO->proceso);
    }
    else
    {
        log_debug(loggerKernel,"## (<%u>) ERROR. PROCESO EN ESTADO INCONSISTENTE",ProcesoEnEsperaIO->proceso->PID);
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

void contadorParaSwap (ProcesoEnEsperaIO* ProcesoEnEsperaIO)
{
    
    usleep(tiempo_suspension*1000); //  *1000 para pasar de milisegundos a microsegundos //TODO ver si hay que pasarlo a microsegundos o como es
    
    //Paso el proceso a Swap
    sem_wait(ProcesoEnEsperaIO->semaforoMutex); 
    pasarASwapBlocked(ProcesoEnEsperaIO);
    log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",ProcesoEnEsperaIO->proceso->PID,"BLOCKED","SWAP_BLOCKED");
    cargarCronometro(ProcesoEnEsperaIO->proceso,BLOCKED);
    sem_post(ProcesoEnEsperaIO->semaforoMutex);
      

                
       
}



void pasarASwapBlocked(ProcesoEnEsperaIO* procesoEsperandoIO)
{
    suspender_proceso_memoria(procesoEsperandoIO->proceso->PID);
    temporal_resume(procesoEsperandoIO->proceso->cronometros[SWAP_BLOCKED]);
    procesoEsperandoIO->proceso->ME[SWAP_BLOCKED]++;
    procesoEsperandoIO->estaENSwap=1;


    sem_post(semaforoInicializarProceso);
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


    sem_post(semaforoInicializarProceso);
}

void manejarFinDeIO(uint32_t PID,char* nombreDispositivoIO,int fdConexion)
{
    sem_wait(semaforoMutexIO);
    bool _esInstancia(InstanciaIO* instanciaIO)
    {
        return instanciaIO->fdConexion == fdConexion;  //Busco la instancia por conexixón, que es lo que las diferencia
    };
    
    DispositivoIO* dispositivoIOLiberado = leerDeDiccionario(diccionarioDispositivosIO,nombreDispositivoIO);
    InstanciaIO* instanciaIO = leerDeListaSegunCondicion(dispositivoIOLiberado->listaInstancias,_esInstancia);

    sem_wait(instanciaIO->semaforoMutex);
        instanciaIO->estaLibre=true;
    sem_post(instanciaIO->semaforoMutex);
    log_info(loggerKernel, "## (<%u>) finalizó IO y pasa a READY",PID);
    
    
    if(!list_is_empty(dispositivoIOLiberado->colaEsperandoIO->lista)) //Si la cola de procesos en espera no esta vacía, empiezo el IO del proceso esperando
        empezarIODelProximoEnEspera(dispositivoIOLiberado);
    
    char* PIDComoChar = pasarUnsignedAChar(PID);
    ProcesoEnEsperaIO* procesoADesbloquear = leerDeDiccionario(diccionarioProcesosBloqueados,PIDComoChar);
    free(PIDComoChar);
    if(procesoADesbloquear!=NULL)
        sem_post(procesoADesbloquear->semaforoIOFinalizada);

    sem_post(semaforoMutexIO);
    

    
}


void empezarIODelProximoEnEspera(DispositivoIO* dispositivoIO)
{
    log_debug(loggerKernel,"## Empezando IO del proximo en espera para el dispositivo: %s",dispositivoIO->nombre); 
    ProcesoEnEsperaIO* ProcesoEnEsperaIO = sacarDeLista(dispositivoIO->colaEsperandoIO,0);
    avisarInicioIO(ProcesoEnEsperaIO,dispositivoIO->nombre,ProcesoEnEsperaIO->tiempo);
}