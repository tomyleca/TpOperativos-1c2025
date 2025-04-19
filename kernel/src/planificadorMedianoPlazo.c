#include "kernel.h"

void pasarABLoqueadoEIniciarContador(PCB* proceso,uint32_t tiempo,char* nombreIO){
    
    //TODO falta sacarlo de la lista donde esta


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


void* contadorParaSwap(char* PID){

    
    t_temporal* contadorEsperaSwap = temporal_create();
    
    
    

    while(1){
        int tiempoTranscurrido = (int) temporal_gettime(contadorEsperaSwap);
        if(tiempoTranscurrido>=4500)
        {
            procesoEnEsperaIO* procesoEsperandoIO = leerDeDiccionario(diccionarioProcesosBloqueados,PID);
            procesoEsperandoIO->estaENSwap=1;
            pasarASwapBlocked(procesoEsperandoIO);
            
        }
        if(IOTerminado(PID))
        {
            //Creo que no hace falta proceso = sacarDeDiccionario(semaforoDiccionarioBlocked,PIDComoChar);
            procesoEnEsperaIO* procesoIOFinalizado = leerDeDiccionario(diccionarioProcesosBloqueados,PID);
            pasarAReady(procesoIOFinalizado->proceso);
            
            break;
        }
    }

    temporal_destroy(contadorEsperaSwap);
    

    return NULL;
}

bool IOTerminado(char* PIDComoChar){
    
    procesoEnEsperaIO* procesoEsperando = leerDeDiccionario( diccionarioProcesosBloqueados, PIDComoChar);
    return procesoEsperando->IOFinalizada;
}

void pasarASwapBlocked(procesoEnEsperaIO* procesoEsperandoIO)
{
    //Le aviso a la memoria que el proceso paso a disco.

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
    if(algoritmoColaNewEnFIFO)
        agregarALista(listaProcesosSwapReady,proceso);
    else 
        agregarAListaOrdenada(listaProcesosSwapReady,proceso,menorTam);
}
