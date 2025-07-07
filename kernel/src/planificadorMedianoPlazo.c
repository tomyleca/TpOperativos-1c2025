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

    
    agregarADiccionario(diccionarioProcesosBloqueados,pasarUnsignedAChar(proceso->PID),procesoEsperando);
    
    


    
    if(avisarInicioIO(procesoEsperando,nombreIO,tiempo) != -1)
    {
        pthread_t hiloManejoBloqueado;
        pthread_create(&hiloManejoBloqueado,NULL,(void *)manejarProcesoBloqueadoPorIO,procesoEsperando);
        procesoEsperando->hiloManejoBloqueado= hiloManejoBloqueado;

        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",proceso->PID,"EXECUTE","BLOCKED");
        cargarCronometro(proceso,EXECUTE);
        suspender_proceso_memoria(proceso->PID);
   
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

    pthread_t hiloContadorSwap;
    pthread_create(&hiloContadorSwap,NULL,(void *)contadorParaSwap,ProcesoEnEsperaIO);
    ProcesoEnEsperaIO->hiloContadorSwap = hiloContadorSwap;


    sem_wait(ProcesoEnEsperaIO->semaforoIOFinalizada);
    
    sacarDeDiccionario(diccionarioProcesosBloqueados,PID);  //Desbloqueo el proceso

    free(PID);
    
    sem_wait(ProcesoEnEsperaIO->semaforoMutex); //Mutex para chequear que el otro hilo no este en medio de un proceso
    esperarCancelacionDeHilo(hiloContadorSwap); //Cancelo el hilo contadorSwap, para que no tire seg fault cuando haga free del semaforoMutex
    
    if(ProcesoEnEsperaIO->estaENSwap == 0) //Chequeo que no se haya pasado a swap
    {

        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",ProcesoEnEsperaIO->proceso->PID,"BLOCKED","READY");
        cargarCronometro(ProcesoEnEsperaIO->proceso,BLOCKED);
        des_suspender_proceso_memoria(ProcesoEnEsperaIO->proceso->PID);
        pasarAReady(ProcesoEnEsperaIO->proceso);
        
        
        free(ProcesoEnEsperaIO->semaforoIOFinalizada);
        free(ProcesoEnEsperaIO->semaforoMutex);
        free(ProcesoEnEsperaIO);
        
    }
    else if(ProcesoEnEsperaIO->estaENSwap == 1)
    {
        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",ProcesoEnEsperaIO->proceso->PID,"SWAP_BLOCKED","SWAP_READY");
        cargarCronometro(ProcesoEnEsperaIO->proceso,SWAP_BLOCKED);
        des_suspender_proceso_memoria(ProcesoEnEsperaIO->proceso->PID);
        pasarASwapReady(ProcesoEnEsperaIO->proceso);
    }
    else
    {
        log_info(loggerKernel,"## (<%u>) ERROR. PROCESO EN ESTADO INCONSISTENTE",ProcesoEnEsperaIO->proceso->PID);
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

void manejarFinDeIO(uint32_t PID,char* nombreDispositivoIO,int fdConexion)
{
    bool _esInstancia(InstanciaIO* instanciaIO)
    {
        return instanciaIO->fdConexion == fdConexion;  //Busco la instancia por conexixón, que es lo que las diferencia
    };
    
    DispositivoIO* dispositivoIOLiberado = leerDeDiccionario(diccionarioDispositivosIO,nombreDispositivoIO);
    InstanciaIO* instanciaIO = leerDeListaSegunCondicion(dispositivoIOLiberado->listaInstancias,_esInstancia);

    sem_wait(instanciaIO->semaforoMutex);
        instanciaIO->estaLibre=true;
    sem_post(instanciaIO->semaforoMutex);

    
    
    if(!list_is_empty(dispositivoIOLiberado->colaEsperandoIO->lista)) //Si la cola de procesos en espera no esta vacía, empiezo el IO del proceso esperando
        empezarIODelProximoEnEspera(dispositivoIOLiberado);
    
    char* PIDComoChar = pasarUnsignedAChar(PID);
    ProcesoEnEsperaIO* procesoADesbloquear = leerDeDiccionario(diccionarioProcesosBloqueados,PIDComoChar);
    free(PIDComoChar);
    sem_post(procesoADesbloquear->semaforoIOFinalizada);

    log_info(loggerKernel, "## (<%u>) finalizó IO y pasa a READY",PID);

    
}

//TODO probarlo
void empezarIODelProximoEnEspera(DispositivoIO* dispositivoIO)
{
    ProcesoEnEsperaIO* ProcesoEnEsperaIO = sacarDeLista(dispositivoIO->colaEsperandoIO,0);
    avisarInicioIO(ProcesoEnEsperaIO,dispositivoIO->nombre,ProcesoEnEsperaIO->tiempo);
}