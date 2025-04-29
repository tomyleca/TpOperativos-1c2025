#include "kernel.h"

void pasarABLoqueado(PCB* proceso,uint32_t tiempo,char* nombreIO){
    
    
    guardarDatosDeEjecucion(proceso);

    
   
    
    procesoEnEsperaIO* procesoEsperando=malloc(sizeof(procesoEnEsperaIO));
    procesoEsperando->proceso=proceso;
    procesoEsperando->estaENSwap=0;
    procesoEsperando->semaforoIOFinalizada=malloc(sizeof(sem_t));
    sem_init(procesoEsperando->semaforoIOFinalizada,1,0);

    char* PIDComoChar = pasarUnsignedAChar(proceso->PID);
    agregarADiccionario(diccionarioProcesosBloqueados,PIDComoChar,procesoEsperando);
    avisarInicioIO(proceso->PID,nombreIO,tiempo);



    pthread_t* hiloContador = malloc(sizeof(pthread_t));
    pthread_create(hiloContador,NULL,manejarProcesoBloqueado,procesoEsperando);
    

    free(PIDComoChar);

}


void* manejarProcesoBloqueado(procesoEnEsperaIO* procesoEnEsperaIO){

    char* PID = pasarUnsignedAChar(procesoEnEsperaIO->proceso->PID);
    temporal_resume(procesoEnEsperaIO->proceso->cronometros[BLOCKED]);
    procesoEnEsperaIO->proceso->ME[BLOCKED]++;
    
    
    

    while(1){
        int64_t tiempoTranscurrido = (int64_t) temporal_gettime(procesoEnEsperaIO->proceso->cronometros[BLOCKED]);
        if(tiempoTranscurrido>=tiempo_suspension)
        {
            //Paso el proceso a Swap
            procesoEnEsperaIO->estaENSwap=1;
            cargarCronometro(procesoEnEsperaIO->proceso,BLOCKED);
            pasarASwapBlocked(procesoEnEsperaIO);
            break;
            
        }
        else if(sem_trywait(procesoEnEsperaIO->semaforoIOFinalizada) == 0)
        {
            //Desbloqueo el proceso
            sacarDeDiccionario(diccionarioProcesosBloqueados,PID);
            cargarCronometro(procesoEnEsperaIO->proceso,BLOCKED);
            pasarAReady(procesoEnEsperaIO->proceso);
            
            free(procesoEnEsperaIO->semaforoIOFinalizada);
            free(procesoEnEsperaIO);
            break;
        }
    }

    
    

    return NULL;
}




void pasarASwapBlocked(procesoEnEsperaIO* procesoEsperandoIO)
{
    //TODO Le aviso a la memoria que el proceso paso a disco.
    
    temporal_resume(procesoEsperandoIO->proceso->cronometros[SWAP_BLOCKED]);
    procesoEsperandoIO->proceso->ME[SWAP_BLOCKED]++;

    sem_wait(procesoEsperandoIO->semaforoIOFinalizada);
    cargarCronometro(procesoEsperandoIO->proceso,SWAP_BLOCKED);
    pasarASwapReady(procesoEsperandoIO->proceso);


    
}

char* pasarUnsignedAChar(uint32_t unsigned_)
{
    //El buffer es de 11 bytes porque para guardar el maximo número representable por un unsigned de 32 bits(4294967295) como caracteres en un char* es de 10 bytes + el '\0'
    char* buffer=malloc(sizeof(12));
    snprintf(buffer,12,"%u",unsigned_);
    return buffer;
}

void pasarASwapReady(PCB* proceso)
{
    temporal_resume(proceso->cronometros[SWAP_READY]);
    proceso->MT[SWAP_READY]++;

    if(algoritmoColaNewEnFIFO)
        agregarALista(listaProcesosSwapReady,proceso);
    else 
        agregarAListaOrdenada(listaProcesosSwapReady,proceso,menorTam);
}

void manejarFinDeIO(uint32_t PID,char* nombreDispositivoIO)
{
    DispositivoIO* dispositivoIOLiberado = leerDeDiccionario(diccionarioDispositivosIO,nombreDispositivoIO);
    sem_post(dispositivoIOLiberado->semaforoDispositivoOcupado);

    
    char* PIDComoChar = pasarUnsignedAChar(PID);
    procesoEnEsperaIO* procesoADesbloquear = leerDeDiccionario(diccionarioProcesosBloqueados,PIDComoChar);
    sem_post(procesoADesbloquear->semaforoIOFinalizada);
    
}