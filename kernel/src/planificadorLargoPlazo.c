#include "kernel.h"

void INIT_PROC(char* archivoPseudocodigo,unsigned int tam){
    //Creo un nuevo proceso
    PCB* nuevoProceso=malloc(sizeof(PCB));
    nuevoProceso->archivoPseudocodigo=archivoPseudocodigo;
    nuevoProceso->tam=tam;
    nuevoProceso->PC=0;
    
    nuevoProceso->PID=pidDisponible;
    
    sem_wait(semaforoPIDDisponible);
        pidDisponible++;
    sem_post(semaforoPIDDisponible);

    

    for(int i=0;i<7;i++)
    {
        nuevoProceso->ME[i]=0;
        nuevoProceso->MT[i]=0;
        nuevoProceso->cronometros[i]=temporal_create();
        temporal_stop(nuevoProceso->cronometros[i]);
    }


    

    nuevoProceso->estimadoRafagaAnterior=0;
    nuevoProceso->duracionRafagaAnterior=0;
    nuevoProceso->estimadoRafagaActual=0;

    
   
    if(algoritmoColaNewEnFIFO)
        agregarALista(listaProcesosNew,nuevoProceso);
    else
        agregarAListaOrdenada(listaProcesosNew,nuevoProceso,menorTam);

    temporal_resume(nuevoProceso->cronometros[NEW]);
    nuevoProceso->ME[NEW]++; 
    
    if(nuevoProceso->PID==0) //Si es el primer proceso, espero el ENTER
    {
        while (1) {
            char* input = readline("Apriete ENTER para empezar a planificar procesos.");  

            if (*input == '\0') {  
                break;
            }
        }
    }    
    
    inicializarProceso();
}

void inicializarProceso(){
    PCB* procesoAInicializar;
    


    if (!list_is_empty(listaProcesosSwapReady->lista)) //Esto es para darle mas prioridad a la lista Swap Ready
        {
        procesoAInicializar= sacarDeLista(listaProcesosSwapReady,0);
        cargarCronometro(procesoAInicializar,SWAP_READY);
        }
    else
        {
        procesoAInicializar = sacarDeLista(listaProcesosNew,0);
        cargarCronometro(procesoAInicializar,NEW);
        }
    
    //TODO LE PREGUNTO A MEMORIA
    
    pasarAReady(procesoAInicializar);
 

    
    


}


bool menorTam(PCB* PCB1,PCB* PCB2)
{
    return PCB1->tam <= PCB2->tam;
}


void pasarAReady(PCB* proceso){
    agregarALista(listaProcesosReady,proceso);
    temporal_resume(proceso->cronometros[READY]);
    proceso->ME[READY]++;
}
