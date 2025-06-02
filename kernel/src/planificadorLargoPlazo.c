#include "kernel.h"

void inicializarProceso(){
    PCB* procesoAInicializar;
    


    if (!list_is_empty(listaProcesosSwapReady->lista)) //Esto es para darle mas prioridad a la lista Swap Ready
        {
        procesoAInicializar= leerDeLista(listaProcesosSwapReady,0);
        
        if(mandarDatosProcesoAMemoria(procesoAInicializar) == OK)
        {   
            sacarDeLista(listaProcesosSwapReady,0);
            log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAInicializar->PID,"SWAP_READY","READY");
            cargarCronometro(procesoAInicializar,SWAP_READY);
            mandarDatosProcesoAMemoria(procesoAInicializar);

            pasarAReady(procesoAInicializar);
           
        }
        }
    else if (!list_is_empty(listaProcesosNew->lista))
        {
        procesoAInicializar = leerDeLista(listaProcesosNew,0);
        
      
        if(mandarDatosProcesoAMemoria(procesoAInicializar) == OK)
        {   
            sacarDeLista(listaProcesosNew,0);
            log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAInicializar->PID,"NEW","READY");
            cargarCronometro(procesoAInicializar,NEW);
            pasarAReady(procesoAInicializar);
        }
        


        
        
        }


}


int mandarDatosProcesoAMemoria(PCB* proceso)
{
    int socketKernelMemoria = crear_conexion(loggerKernel,ip_memoria,puerto_memoria);
    t_paquete* paquete = crear_super_paquete(GUARDAR_PROCESO_EN_MEMORIA);
    cargar_uint32_t_al_super_paquete(paquete,proceso->PID);
    cargar_uint32_t_al_super_paquete(paquete,proceso->tam);
    cargar_string_al_super_paquete(paquete,proceso->archivoPseudocodigo);
    enviar_paquete(paquete,socketKernelMemoria);
    int respuesta = recibir_operacion(socketKernelMemoria);
    
    if(respuesta == NO_HAY_MEMORIA)
    {
        log_error(loggerKernel,"## <%u> No hay suficiente memoria para alojar el proceso",proceso->PID);
    }

    free(paquete);
    close(socketKernelMemoria);

    return respuesta;
    
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
