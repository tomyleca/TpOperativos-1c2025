#include "conexiones_kernel.h"

pthread_t escuchar_dispatch_cpu()
{
    pthread_t hilo_dispatch_cpu;
    pthread_create(&hilo_dispatch_cpu, NULL, (void*) atender_dispatch_cpu, NULL);
    return hilo_dispatch_cpu;
}

pthread_t escuchar_interrupcion_cpu()
{
    pthread_t hilo_interrupcion_cpu;
    pthread_create(&hilo_interrupcion_cpu, NULL, (void*) atender_interrupcion_cpu, NULL);
    return hilo_interrupcion_cpu;
}

pthread_t crear_hilo_memoria()
{
    pthread_t hilo_kernel_memoria;
    pthread_create(&hilo_kernel_memoria, NULL, (void*) hilo_memoria, NULL);
    return hilo_kernel_memoria;
}

void hilo_memoria() 
{
    t_buffer* buffer;
    int cod_op;
    printf("LLegue a hilo_memoria");
    while(1) 
    {
        cod_op = recibir_operacion(socket_kernel_memoria);
        switch (cod_op) 
        {
            case MENSAJE:
                //recibir_mensaje(socket_kernel_cpu_dispatch, loggerKernel);
                break;            
            case -1:
                log_error(loggerKernel, "KERNEL DISPATCH se desconecto. Terminando servidor");
                pthread_exit(NULL);
            /*default:
                log_warning(loggerKernel, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}


void atender_dispatch_cpu() 
{
    t_buffer* buffer;
    int cod_op;
    cliente_kernel_dispatch = esperar_cliente(socket_kernel_cpu_dispatch);
    t_paquete* paquete;
    log_info(loggerKernel, "Se conectó DISPATCH");
    while(1) 
    {
        cod_op = recibir_operacion(cliente_kernel_dispatch);
        switch (cod_op) 
        {
            case MENSAJE:
                //recibir_mensaje(cliente_kernel_dispatch, loggerKernel);
                break;
            case HANDSHAKE_CPU_KERNEL_D:
            buffer = recibiendo_super_paquete(cliente_kernel_dispatch);
            char* identificador2 = recibir_string_del_buffer(buffer);    
            enviar_pid_contexto_cpu(cliente_kernel_dispatch); 
            break;
            case -1:
                log_error(loggerKernel, "KERNEL DISPATCH se desconecto. Terminando servidor");
                pthread_exit(NULL);
            default:
                log_warning(loggerKernel, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}

void atender_interrupcion_cpu() 
{
    t_buffer* buffer;
    int cod_op;
    cliente_kernel_interrupt = esperar_cliente(socket_kernel_cpu_interrupt);
    log_info(loggerKernel, "Se conectó INTERRUPT");
    t_paquete* paquete;

    while(1) 
    {
        cod_op = recibir_operacion(cliente_kernel_interrupt);
        switch (cod_op) 
        {
            case MENSAJE:
                //recibir_mensaje(cliente_kernel_interrupt, loggerKernel);
                break;
            case HANDSHAKE_CPU_KERNEL_I:
            buffer = recibiendo_super_paquete(cliente_kernel_interrupt);
            char* identificador = recibir_string_del_buffer(buffer);
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

void ejecutar_io() 
{
    t_buffer* buffer;
    int cod_op;
    
    while(1) 
    {
        cod_op = recibir_operacion(socket_kernel_io);
        switch (cod_op) 
        {
            case MENSAJE:
                //recibir_mensaje(socket_kernel_io, loggerKernel);
                break;
            case -1:
                log_error(loggerKernel, "KERNEL se desconecto. Terminando servidor");
                pthread_exit(NULL);
            default:
                log_warning(loggerKernel, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}


void enviar_pid_contexto_cpu(int cliente_kernel_dispatch)
{
    t_paquete* paquete; 
    int uno =1;
    int cero=0;
    paquete = crear_super_paquete(PID_KERNEL_A_CPU);
    cargar_int_al_super_paquete(paquete, uno);
    cargar_int_al_super_paquete(paquete, cero);
    enviar_paquete(paquete, cliente_kernel_dispatch);
    free(paquete);
}
