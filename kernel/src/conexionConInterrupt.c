#include "conexionConCPU.h"

void* esperarClientesInterrupt(void* arg)
{
    while(1)
    {
        int* fdConexion = malloc(sizeof(int));
        *fdConexion = esperar_cliente(socket_kernel_cpu_interrupt);
        log_info(loggerKernel, "## Se conectó INTERRUPT");
        pthread_t hilo_interrupcion_cpu;
        pthread_create(&hilo_interrupcion_cpu, NULL, (void*) atender_interrupcion_cpu, fdConexion);
    }
}


void atender_interrupcion_cpu(void* conexion) 
{
    int fdConexion = *(int*) conexion;
    
    t_buffer* buffer;
    op_code cod_op;
    
    nucleoCPU* nucleoCPU;
    
   

    while(1) 
    {
        cod_op = recibir_operacion(fdConexion);
        switch (cod_op) 
        {
            case MENSAJE:
                //recibir_mensaje(*fdConexion, loggerKernel,);
                break;
            case HANDSHAKE_CPU_KERNEL_I:
                buffer = recibiendo_super_paquete(fdConexion);
                char* identificador = recibir_string_del_buffer(buffer);
                nucleoCPU = guardarDatosCPUInterrupt(identificador,fdConexion);
            break;
            /*case -1:
                log_error(loggerKernel, "KERNEL INTERRUPT se desconecto. Terminando servidor");
                pthread_exit(NULL);*/
            /*default:
                log_warning(loggerKernel, "Operacion desconocida. No quieras meter la pata");
                break;*/
        }
    }
}

nucleoCPU* guardarDatosCPUInterrupt(char* identificador,int fdConexion)
{
    sem_wait(semaforoGuardarDatosCPU);
    
    nucleoCPU* nucleoCPU = chequearSiCPUYaPuedeInicializarse(identificador);
    if(nucleoCPU == NULL)//Si esta en NULL quiere decir que la otra conexion todavía no llego
    {
        nucleoCPU = malloc(sizeof(nucleoCPU));
        nucleoCPU->identificador= malloc(strlen(identificador));
        strcpy(nucleoCPU->identificador,identificador);
        nucleoCPU->procesoEnEjecucion=NULL;
        nucleoCPU->fdConexionInterrupt = fdConexion;
        agregarALista(listaCPUsAInicializar,nucleoCPU);
    }
    else
    {
        sacarElementoDeLista(listaCPUsAInicializar,nucleoCPU);
        agregarALista(listaCPUsLibres,nucleoCPU);
        nucleoCPU->fdConexionInterrupt = fdConexion;
        sem_post(semaforoIntentarPlanificar);
    }

    sem_post(semaforoGuardarDatosCPU);

    return nucleoCPU;

}