#include "conexionConCPU.h"

void* esperarClientesDispatch(void* arg)
{
    while(1)
    {
        int* fdConexion = malloc(sizeof(int));
        *fdConexion = esperar_cliente(socket_kernel_cpu_dispatch);
        log_info(loggerKernel, "## Se conectó DISPATCH");
        pthread_t hilo_dispatch_cpu;
        pthread_create(&hilo_dispatch_cpu, NULL, (void*) atender_dispatch_cpu, fdConexion);
    }
}

void atender_dispatch_cpu(void* conexion) 
{
    int fdConexion = *(int*)conexion;
    
    t_buffer* buffer;
    int cod_op;
    nucleoCPU* nucleoCPUEnEjecucion = malloc(sizeof(nucleoCPUEnEjecucion));
    
    
   
    
    while(1) 
    {
        cod_op = recibir_operacion(fdConexion);
        switch (cod_op) 
        {
            case MENSAJE:
                //recibir_mensaje(*fdConexion, loggerKernel);
                break;
            case HANDSHAKE_CPU_KERNEL_D:
                buffer = recibiendo_super_paquete(fdConexion);
                char* identificador = recibir_string_del_buffer(buffer);    
                guardarDatosCPU(identificador,fdConexion,nucleoCPUEnEjecucion);
                break;
            case -1:
                log_error(loggerKernel, "KERNEL DISPATCH se desconecto. Terminando servidor");
                pthread_exit(NULL);
            default:
                //log_warning(loggerKernel, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}



void guardarDatosCPU(char* identificador,int fdConexion,nucleoCPU* nuevoNucleoCPU)
{
 
    nuevoNucleoCPU->identificador= malloc(strlen(identificador));
    nuevoNucleoCPU->identificador=identificador;
    
    nuevoNucleoCPU->procesoEnEjecucion=NULL;
    nuevoNucleoCPU->fdConexion = fdConexion;

   
   

    agregarALista(listaCPUsLibres,nuevoNucleoCPU);
    sem_post(semaforoIntentarPlanificar);

    

}




void pasarAExit(PCB* proceso){
    log_info(loggerKernel,"## (<%u>) - Finaliza el proceso",proceso->PID);
    log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",proceso->PID,"EXECUTE","EXIT");
    //TODO avisar a memoria 
    proceso->ME[EXIT]++;
    loggearMetricas(proceso);
    inicializarProceso();
    //TODO liberar cronometros
    free(proceso->archivoPseudocodigo);
    free(proceso);
}

void loggearMetricas(PCB* proceso)
{   
    log_info(loggerKernel,"## (<%u>) - Métricas de estado: NEW (%d) (%d), READY (%d) (%d) EXECUTE (%d) (%d) BLOCKED (%d) (%d) SWAP_BLOCKED (%d) (%d) SWAP_READY (%d) (%d) EXIT (%d) (%d)",
    proceso->PID,
    proceso->ME[NEW],proceso->MT[NEW],
    proceso->ME[READY],proceso->MT[READY],
    proceso->ME[EXECUTE],proceso->MT[EXECUTE],
    proceso->ME[BLOCKED],proceso->MT[BLOCKED],
    proceso->ME[SWAP_BLOCKED],proceso->MT[SWAP_BLOCKED],
    proceso->ME[SWAP_READY],proceso->MT[SWAP_READY],
    proceso->ME[EXIT],proceso->MT[EXIT]);
}



void mandarContextoACPU(uint32_t PID,uint32_t PC,int fdConexion)
{
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = PID_KERNEL_A_CPU;
    cargar_uint32_t_al_super_paquete(paquete,PID);
    cargar_uint32_t_al_super_paquete(paquete,PC);
    enviar_paquete(paquete,fdConexion);
    esperarOK(fdConexion);
    free(paquete);
}