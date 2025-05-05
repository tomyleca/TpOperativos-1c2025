#include "kernel.h"

void INIT_PROC(char* archivoPseudocodigo,unsigned int tam){
    //Creo un nuevo proceso
    PCB* nuevoProceso=malloc(sizeof(PCB));
    nuevoProceso->archivoPseudocodigo=archivoPseudocodigo;
    nuevoProceso->tam=tam;
    nuevoProceso->PC=0;
    


    nuevoProceso->PID=pidDisponible;  
    
    t_paquete* paquete = crear_super_paquete(RECIBIR_PID_KERNEL);
    cargar_int_al_super_paquete(paquete, nuevoProceso->PID);
    cargar_int_al_super_paquete(paquete, nuevoProceso->tam);
    cargar_string_al_super_paquete(paquete, nuevoProceso->archivoPseudocodigo);
    enviar_paquete(paquete, socket_kernel_memoria);
    pidDisponible++;
    

    for(int i=0;i<7;i++)
    {
        nuevoProceso->ME[i]=0;
        nuevoProceso->MT[i]=0;
        nuevoProceso->cronometros[i]=temporal_create();
        temporal_stop(nuevoProceso->cronometros[i]);
    }


    nuevoProceso->ME[NEW]++;

    nuevoProceso->estimadoRafagaAnterior=0;
    nuevoProceso->duracionRafagaAnterior=0;
    nuevoProceso->estimadoRafagaActual=0;
   
    if(algoritmoColaNewEnFIFO)
        agregarALista(listaProcesosNew,nuevoProceso);
    else
        agregarAListaOrdenada(listaProcesosNew,nuevoProceso,menorTam);

    temporal_resume(nuevoProceso->cronometros[NEW]);
    inicializarProceso();
}

void inicializarProceso(){
    PCB* procesoAInicializar;
    
<<<<<<< HEAD


    if (!list_is_empty(listaProcesosSwapReady->lista)) //Esto es para darle mas prioridad a la lista Swap Ready
        {
        procesoAInicializar= sacarDeLista(listaProcesosSwapReady,0);
        
        
        //TODO Preguntarle a memoria
        log_info(logger_kernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAInicializar->PID,"SWAP_READY","READY");
        cargarCronometro(procesoAInicializar,SWAP_READY);
        pasarAReady(procesoAInicializar);
        }
=======
    //TODO
    //ACA VA ALGO PARA ESPERAR EL ENTER
    if (!list_is_empty(listaProcesosSwapReady))
        procesoAInicializar= sacarDeLista(semaforoListaSwapReady,listaProcesosSwapReady,0);
>>>>>>> origin/CPU-Eze
    else
        {
        procesoAInicializar = sacarDeLista(listaProcesosNew,0);

        
        //TODO Preguntarle a memoria
        log_info(logger_kernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAInicializar->PID,"NEW","READY");
        cargarCronometro(procesoAInicializar,NEW);
        pasarAReady(procesoAInicializar);
        }
    
    
    
    
    
 

    
    


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
