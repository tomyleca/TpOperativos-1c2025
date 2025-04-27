#include "kernel.h"

void pasarABLoqueadoEIniciarContador(PCB* proceso,uint32_t tiempo,char* nombreIO){
    
    //TODO falta sacarlo de la lista donde esta
    guardarDatosDeEjecucion(proceso);

    avisarInicioIO(proceso->PID,nombreIO,tiempo);
   
    
    procesoEnEsperaIO* procesoEsperando=malloc(sizeof(procesoEnEsperaIO));
    procesoEsperando->proceso=proceso;
    procesoEsperando->estaENSwap=0;
    procesoEsperando->IOFinalizada=0;
    procesoEsperando->semaforoIOFinalizada=malloc(sizeof(sem_t));
    sem_init(procesoEsperando->semaforoIOFinalizada,1,0);

    char* PIDComoChar = pasarUnsignedAChar(proceso->PID);
    agregarADiccionario(diccionarioProcesosBloqueados,PIDComoChar,procesoEsperando);



    pthread_t* hiloContador = malloc(sizeof(pthread_t));
    pthread_create(hiloContador,NULL,contadorParaSwap,proceso);

    
    procesoEsperando = sacarDeDiccionario(diccionarioProcesosBloqueados,PIDComoChar);

    free(PIDComoChar);
    free(procesoEsperando->semaforoIOFinalizada);
    free(procesoEsperando);




}


void* contadorParaSwap(PCB* proceso){

    char* PID = pasarUnsignedAChar(proceso->PID);
    temporal_resume(proceso->cronometros[BLOCKED]);
    proceso->ME[BLOCKED]++;
    
    
    

    while(1){
        int64_t tiempoTranscurrido = (int64_t) temporal_gettime(proceso->cronometros[BLOCKED]);
        if(tiempoTranscurrido>=4500)
        {
            procesoEnEsperaIO* procesoEsperandoIO = leerDeDiccionario(diccionarioProcesosBloqueados,PID);
            procesoEsperandoIO->estaENSwap=1;
            cargarCronometro(proceso,BLOCKED);
            pasarASwapBlocked(procesoEsperandoIO);
            
        }
        else if(IOTerminado(PID))
        {
            //Creo que no hace falta proceso = sacarDeDiccionario(semaforoDiccionarioBlocked,PIDComoChar);
            procesoEnEsperaIO* procesoIOFinalizado = leerDeDiccionario(diccionarioProcesosBloqueados,PID);
            pasarAReady(procesoIOFinalizado->proceso);
            
            break;
        }
    }

    
    

    return NULL;
}

bool IOTerminado(char* PIDComoChar){
    
    procesoEnEsperaIO* procesoEsperando = leerDeDiccionario( diccionarioProcesosBloqueados, PIDComoChar);
    return procesoEsperando->IOFinalizada;
}

void pasarASwapBlocked(procesoEnEsperaIO* procesoEsperandoIO)
{
    //TODO Le aviso a la memoria que el proceso paso a disco.
    
    temporal_resume(procesoEsperandoIO->proceso->cronometros[SWAP_BLOCKED]);
    procesoEsperandoIO->proceso->ME[SWAP_BLOCKED]++;

    sem_wait(procesoEsperandoIO->semaforoIOFinalizada);
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
