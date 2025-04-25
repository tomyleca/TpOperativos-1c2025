#include "kernel.h"

void INIT_PROC(char* archivoPseudocodigo,unsigned int tam){
    //Creo un nuevo proceso
    PCB* nuevoProceso=malloc(sizeof(PCB));
    nuevoProceso->archivoPseudocodigo=archivoPseudocodigo;
    nuevoProceso->tam=tam;
    nuevoProceso->PC=0;
    
    nuevoProceso->PID=pidDisponible;
    pidDisponible++;
    

    for(int i=0;i<7;i++)
    {
        nuevoProceso->ME[i]=0;
        nuevoProceso->MT[i]=0;
    }

    nuevoProceso->ME[NEW]++;

    nuevoProceso->estimadoRafagaAnterior=0;
    nuevoProceso->duracionRafagaAnterior=0;
    nuevoProceso->estimadoRafagaActual=0;
   
    if(algoritmoColaNewEnFIFO)
        agregarALista(listaProcesosNew,nuevoProceso);
    else
        agregarAListaOrdenada(listaProcesosNew,nuevoProceso,menorTam);

    
    inicializarProceso();
}

void inicializarProceso(){
    PCB* procesoAInicializar;
    
    //TODO
    //ACA VA ALGO PARA ESPERAR EL ENTER
    if (!list_is_empty(listaProcesosSwapReady->lista))
        procesoAInicializar= sacarDeLista(listaProcesosSwapReady,0);
    else
        procesoAInicializar = sacarDeLista(listaProcesosNew,0);
    
    //TODO
    //LE PREGUNTO A MEMORIA
    pasarAReady(procesoAInicializar);

    
    


}

bool menorTam(PCB* PCB1,PCB* PCB2)
{
    return PCB1->tam <= PCB2->tam;
}


void pasarAReady(PCB* proceso){
    agregarALista(listaProcesosReady,proceso);
    proceso->ME[READY]++;
}