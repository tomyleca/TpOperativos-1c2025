#include "conexionConCPU.h"

void* esperarClientesInterrupt(void* arg)
{
    while(1)
    {
        int* fdConexion = malloc(sizeof(int));
        *fdConexion = esperar_cliente(socket_kernel_cpu_interrupt);
        log_info(loggerKernel, "## Se conectó INTERRUPT");
        pthread_t hilo_interrupcion_cpu;
        pthread_create(&hilo_interrupcion_cpu, NULL, (void*) esperarDatosInterrupt, fdConexion);
    }
}


void esperarDatosInterrupt(void* conexion) 
{
    int fdConexion = *(int*) conexion;
    
    t_buffer* buffer;
    int cod_op;
    
    
    while(1)
    {
    cod_op = recibir_operacion(fdConexion);
    switch(cod_op)    
    {
        case 1:
            printf("Llega OK interrupt");
            sem_post(semaforoEsperarOKInterrupt);
            break;
        case HANDSHAKE_CPU_KERNEL_I:
            buffer = recibiendo_super_paquete(fdConexion);
            char* identificador = recibir_string_del_buffer(buffer);
            guardarDatosCPUInterrupt(identificador,fdConexion);
            limpiarBuffer(buffer);
            break;
        
        case -1:
            log_info(loggerKernel,"# Se desconectó Interrupt");
            //shutdown(fdConexion, SHUT_RDWR);
            close(fdConexion);
            pthread_exit(NULL);
            break;
        default:
            log_info(loggerKernel,"# Operación desconocida en Interrupt");
            break;

    
    }
    }
}

NucleoCPU* guardarDatosCPUInterrupt(char* identificador,int fdConexion)
{
    sem_wait(semaforoMutexGuardarDatosCPU);
    
    NucleoCPU* nucleoCPU = chequearSiCPUYaPuedeInicializarse(identificador);
    if(nucleoCPU == NULL)//Si esta en NULL quiere decir que la otra conexion todavía no llego
    {
        nucleoCPU = malloc(sizeof(*nucleoCPU));
        nucleoCPU->identificador= malloc(strlen(identificador) + 1);
        strcpy(nucleoCPU->identificador,identificador);
        free(identificador);
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

    sem_post(semaforoMutexGuardarDatosCPU);

    return nucleoCPU;

}

void mandarInterrupcion(NucleoCPU* nucleoCPU)
{
    int Interrupcion = INTERRUPCION;
    send(nucleoCPU->fdConexionInterrupt,&Interrupcion,sizeof(int),0);   
    sem_wait(semaforoEsperarOKInterrupt);
}