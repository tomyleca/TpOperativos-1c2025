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
    


    sem_wait(semaforoMutexIO);
        if(avisarInicioIO(procesoEsperando,nombreIO,tiempo) != -1)
        {
            sem_post(semaforoMutexIO);
            log_info(loggerKernel, "## (<%u>) - Bloqueado por IO: <%s>",procesoEsperando->proceso->PID,nombreIO);
            pthread_t hiloContadorSwap;
            pthread_create(&hiloContadorSwap,NULL,contadorParaSwap,procesoEsperando);
            pthread_detach(hiloContadorSwap);
            procesoEsperando->hiloContadorSwap = hiloContadorSwap;
            pthread_t hiloManejoBloqueado;
            pthread_create(&hiloManejoBloqueado,NULL,manejarProcesoBloqueadoPorIO,procesoEsperando);
            pthread_detach(hiloManejoBloqueado);
            procesoEsperando->hiloManejoBloqueado= hiloManejoBloqueado;


            

            log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",proceso->PID,"EXECUTE","BLOCKED");
            
    
        }
        else
        {
            sem_post(semaforoMutexIO);
            log_error(loggerKernel, "## (<%u>) - Dispositivo IO %s no encontrado. Finalizando proceso", procesoEsperando->proceso->PID, nombreIO);
            pasarAExit(procesoEsperando->proceso,"EXECUTE");
            free(procesoEsperando->semaforoMutex);
            free(procesoEsperando->semaforoIOFinalizada);
            free(procesoEsperando);
        }
    
    
    
}


void* manejarProcesoBloqueadoPorIO(void* arg){
    
    ProcesoEnEsperaIO* procesoEnEsperaIO = (ProcesoEnEsperaIO*) arg;
    char* PID = pasarUnsignedAChar(procesoEnEsperaIO->proceso->PID);
 
    temporal_resume(procesoEnEsperaIO->proceso->cronometros[BLOCKED]);
    procesoEnEsperaIO->proceso->ME[BLOCKED]++;




    sem_wait(procesoEnEsperaIO->semaforoIOFinalizada);
    
    sacarDeDiccionario(diccionarioProcesosBloqueados,PID);  //Desbloqueo el proceso

    free(PID);
    
    sem_wait(procesoEnEsperaIO->semaforoMutex); //Mutex para chequear que el otro hilo no este en medio de un proceso
    
    
    if(procesoEnEsperaIO->estaENSwap == 0) //Chequeo que no se haya pasado a swap
    {
        if(procesoEnEsperaIO->proceso == NULL) //quiere decir que el proceso ya paso a exit
        {
            sem_destroy(procesoEnEsperaIO->semaforoIOFinalizada);
            free(procesoEnEsperaIO->semaforoIOFinalizada);
            procesoEnEsperaIO->semaforoIOFinalizada = NULL;
            sem_post(procesoEnEsperaIO->semaforoMutex); //le paso la pelota para que termine de hacer los frees
            return NULL;
        }



        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoEnEsperaIO->proceso->PID,"BLOCKED","READY");
        cargarCronometro(procesoEnEsperaIO->proceso,BLOCKED);
        pasarAReady(procesoEnEsperaIO->proceso,false);
        
        sem_destroy(procesoEnEsperaIO->semaforoIOFinalizada);
        free(procesoEnEsperaIO->semaforoIOFinalizada);
        procesoEnEsperaIO->semaforoIOFinalizada = NULL;
        sem_post(procesoEnEsperaIO->semaforoMutex); //le paso la pelota para que termine de hacer los frees
        
    }
    else if(procesoEnEsperaIO->estaENSwap == 1)
    {
        if(procesoEnEsperaIO->proceso == NULL) //quiere decir que el proceso ya paso a exit
        {
            sem_destroy(procesoEnEsperaIO->semaforoIOFinalizada);
            free(procesoEnEsperaIO->semaforoIOFinalizada);
            procesoEnEsperaIO->semaforoIOFinalizada = NULL;
            sem_destroy(procesoEnEsperaIO->semaforoMutex);
            free(procesoEnEsperaIO->semaforoMutex);
            free(procesoEnEsperaIO);
            return NULL;
        }
        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoEnEsperaIO->proceso->PID,"SWAP_BLOCKED","SWAP_READY");
        cargarCronometro(procesoEnEsperaIO->proceso,SWAP_BLOCKED);
        pasarASwapReady(procesoEnEsperaIO->proceso);

        sem_destroy(procesoEnEsperaIO->semaforoIOFinalizada);
        free(procesoEnEsperaIO->semaforoIOFinalizada);
        procesoEnEsperaIO->semaforoIOFinalizada = NULL;
        sem_destroy(procesoEnEsperaIO->semaforoMutex);
        free(procesoEnEsperaIO->semaforoMutex);
        free(procesoEnEsperaIO);
    }
    else
    {
        log_debug(loggerKernel,"## (<%u>) ERROR. PROCESO EN ESTADO INCONSISTENTE",procesoEnEsperaIO->proceso->PID);
        exit(1);
    }
    
    
    

    

    
    

    return NULL;
}



void* contadorParaSwap (void* arg)
{
    ProcesoEnEsperaIO* procesoEnEsperaIO = (ProcesoEnEsperaIO*) arg;
    usleep(tiempo_suspension*1000); //  *1000 para pasar de milisegundos a microsegundos //TODO ver si hay que pasarlo a microsegundos o como es
    
    sem_wait(procesoEnEsperaIO->semaforoMutex); 

        if(procesoEnEsperaIO->proceso == NULL) //quiere decir que ya finalizo el proceso
        {
            sem_destroy(procesoEnEsperaIO->semaforoIOFinalizada);
            free(procesoEnEsperaIO->semaforoIOFinalizada);
            procesoEnEsperaIO->semaforoIOFinalizada = NULL;
            sem_destroy(procesoEnEsperaIO->semaforoMutex);
            free(procesoEnEsperaIO->semaforoMutex);
            free(procesoEnEsperaIO);
            return NULL;        
        }
    
    
        if(procesoEnEsperaIO->semaforoIOFinalizada == NULL)  //QUIERE DECIR QUE YA LO PASO A READY
        {
            sem_destroy(procesoEnEsperaIO->semaforoMutex);
            free(procesoEnEsperaIO->semaforoMutex);
            free(procesoEnEsperaIO);
            return NULL;
        }
        
        //Paso el proceso a Swap
        pasarASwapBlocked(procesoEnEsperaIO);
        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoEnEsperaIO->proceso->PID,"BLOCKED","SWAP_BLOCKED");
        cargarCronometro(procesoEnEsperaIO->proceso,BLOCKED);
        
        
    sem_post(procesoEnEsperaIO->semaforoMutex);

    return NULL;
      

                
       
}



void pasarASwapBlocked(ProcesoEnEsperaIO* procesoEsperandoIO)
{
    suspender_proceso_memoria(procesoEsperandoIO->proceso->PID);
    temporal_resume(procesoEsperandoIO->proceso->cronometros[SWAP_BLOCKED]);
    procesoEsperandoIO->proceso->ME[SWAP_BLOCKED]++;
    procesoEsperandoIO->estaENSwap=1;


    
}


void pasarASwapReady(PCB* proceso)
{
    temporal_resume(proceso->cronometros[SWAP_READY]);
    proceso->ME[SWAP_READY]++;

    if(algoritmoColaNewEnFIFO)
    {
        agregarALista(listaProcesosSwapReady,proceso);
    }
    else 
    {
        agregarAListaOrdenada(listaProcesosSwapReady,proceso,menorTam);
    }


    int valorSemaforo;
    sem_getvalue(semaforoInicializarProceso,&valorSemaforo);
    

    if(valorSemaforo<=0) 
        sem_post(semaforoInicializarProceso);
}

void manejarFinDeIO(uint32_t PID,char* nombreDispositivoIO,int fdConexion)
{
    sem_wait(semaforoMutexIO);   
        bool _esInstancia(void* arg)
        {
            InstanciaIO* instanciaIO = (InstanciaIO*) arg;
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