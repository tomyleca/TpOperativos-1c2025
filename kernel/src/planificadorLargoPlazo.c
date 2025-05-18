#include "kernel.h"

void inicializarProceso(){
    PCB* procesoAInicializar;
    


    if (!list_is_empty(listaProcesosSwapReady->lista)) //Esto es para darle mas prioridad a la lista Swap Ready
        {
        procesoAInicializar= sacarDeLista(listaProcesosSwapReady,0);
        
        int socketKernelMemoria = crear_conexion(loggerKernel,ip_memoria,puerto_memoria);

        t_paquete* paquete = crear_super_paquete(CHEQUEAR_SI_HAY_MEMORIA_LIBRE);
        cargar_uint32_t_al_super_paquete(paquete,procesoAInicializar->tam);
        enviar_paquete(paquete,socketKernelMemoria);

        if(recibir_operacion(socketKernelMemoria)==OK)
        {   
            log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAInicializar->PID,"SWAP_READY","READY");
            cargarCronometro(procesoAInicializar,SWAP_READY);
            mandarDatosProcesoAMemoria(procesoAInicializar);

            pasarAReady(procesoAInicializar);
           
        }


        
        free(paquete);
        close(socketKernelMemoria);
        }
    else if (!list_is_empty(listaProcesosNew->lista))
        {
        procesoAInicializar = sacarDeLista(listaProcesosNew,0);
        
        int socketKernelMemoria = crear_conexion(loggerKernel,ip_memoria,puerto_memoria);

        t_paquete* paquete = crear_super_paquete(CHEQUEAR_SI_HAY_MEMORIA_LIBRE);
        cargar_uint32_t_al_super_paquete(paquete,procesoAInicializar->tam);
        enviar_paquete(paquete,socketKernelMemoria);

        if(recibir_operacion(socketKernelMemoria)==OK)
        {   
            mandarDatosProcesoAMemoria(procesoAInicializar);
            log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAInicializar->PID,"NEW","READY");
            cargarCronometro(procesoAInicializar,NEW);
         
         
            pasarAReady(procesoAInicializar);

            
        }


        free(paquete);
        close(socketKernelMemoria);
        }


}


void mandarDatosProcesoAMemoria(PCB* proceso)
{
    int socketKernelMemoria = crear_conexion(loggerKernel,ip_memoria,puerto_memoria);
    t_paquete* paquete = crear_super_paquete(RECIBIR_PID_KERNEL);
    cargar_uint32_t_al_super_paquete(paquete,proceso->PID);
    cargar_uint32_t_al_super_paquete(paquete,proceso->tam);
    cargar_string_al_super_paquete(paquete,proceso->archivoPseudocodigo);
    enviar_paquete(paquete,socketKernelMemoria);
    
    if(!esperarOK2(socketKernelMemoria))
    {
        log_error(loggerKernel,"## <%u> Memoria no pudo guardar los datos del proceso",proceso->PID);
        exit(1);
    }

    free(paquete);
    close(socketKernelMemoria);
    
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
