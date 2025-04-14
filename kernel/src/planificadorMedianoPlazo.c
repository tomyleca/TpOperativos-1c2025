#include "kernel.h"

void pasarABLoqueadoEIniciarContador(PCB* proceso){
    
    //TODO
    //falta sacarlo de la lista donde esta

    //Si hace falta lo agrego a un diccionario, creo que no
    

    pthread_t* hiloContador = malloc(sizeof(pthread_t));
    pthread_create(hiloContador,NULL,contadorParaSwap,proceso);




}

void contadorParaSwap(PCB* proceso){

    
    t_temporal* contadorEsperaSwap = temporal_create();
    
    
    char* PIDComoChar = pasarUnsignedAChar(proceso->PID);

    while(1){
        int tiempoTranscurrido = (int) temporal_gettime(contadorEsperaSwap);
        if(tiempoTranscurrido>=4500)
        {
            pasarASwapBlocked(proceso,PIDComoChar);
            
        }
        if(IOTerminado(PIDComoChar))
        {
            //Creo que no hace falta proceso = sacarDeDiccionario(semaforoDiccionarioBlocked,PIDComoChar);
            
            pasarAReady(proceso);
            
            break;
        }
    }

    temporal_destroy(contadorEsperaSwap);
    free(PIDComoChar);
}

bool IOTerminado(char* PIDComoChar){
    
    bool estadoIO = leerDeDiccionario(semaforoDiccionarioIOBlocked, diccionarioIODeProcesosBloqueados, PIDComoChar);
    return estadoIO;
}

void pasarASwapBlocked(PCB* proceso,char* PIDComoChar)
{
    //Creo que no hace falta agregarADiccionario(semaforoDiccionarioBlockedSwap,diccionarioProcesosSwapBloqueados,PIDComoChar,proceso);

    //Le aviso a la memoria que el proceso paso a disco.

    //Este while se podría mejorar con un semaforo, pero tendría que cambiar la forma en la que los dispositivos IO le avisan a los procesos que ya terminaron. Enrealidad tendría que tener dos implementaciones de la misma.
    while(!IOTerminado(PIDComoChar));

    pasarASwapReady(proceso);

    //Aca lo agrego a la cola de new con mas prioridad
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
    agregarALista(semaforoListaSwapReady,listaProcesosSwapReady,proceso);
}
