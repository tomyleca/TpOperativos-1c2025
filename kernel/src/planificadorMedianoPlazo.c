#include "kernel.h"

void pasarABLoqueadoEIniciarContador(PCB* proceso){
    
    //TODO
    //falta sacarlo de la lista donde esta

    //Si hace falta lo agrego a un diccionario, creo que no
    

    pthread_t* hiloContador = malloc(sizeof(pthread_t));
    pthread_create(hiloContador,NULL,contadorParaSwap,proceso);




}

void contadorParaSwap(PCB* proceso){

    t_temporal* contadorEsperaSwap = malloc(sizeof(t_temporal));
    temporal_create(contadorEsperaSwap);
    
    char* PIDComoChar = itoa(PCB->PID);

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

    //Aca lo agrego a la cola de new con mas prioridad
}