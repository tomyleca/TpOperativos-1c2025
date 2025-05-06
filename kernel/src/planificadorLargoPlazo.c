#include "kernel.h"

void inicializarProceso(){
    PCB* procesoAInicializar;
    


    if (!list_is_empty(listaProcesosSwapReady->lista)) //Esto es para darle mas prioridad a la lista Swap Ready
        {
        procesoAInicializar= sacarDeLista(listaProcesosSwapReady,0);
        
        
        //TODO Preguntarle a memoria
        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAInicializar->PID,"SWAP_READY","READY");
        cargarCronometro(procesoAInicializar,SWAP_READY);
        pasarAReady(procesoAInicializar);
        }
    else if (!list_is_empty(listaProcesosNew->lista))
        {
        procesoAInicializar = sacarDeLista(listaProcesosNew,0);

        
        //TODO Preguntarle a memoria
        log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAInicializar->PID,"NEW","READY");
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
