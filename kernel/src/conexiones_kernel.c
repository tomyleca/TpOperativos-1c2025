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
    while(1) 
    {
        cod_op = recibir_operacion(socket_kernel_cpu_dispatch);
        switch (cod_op) 
        {
            case MENSAJE:
                //recibir_mensaje(socket_kernel_cpu_dispatch, logger_kernel);
                break;
            /*case -1:
                log_error(logger_kernel, "KERNEL DISPATCH se desconecto. Terminando servidor");
                pthread_exit(NULL);*/
            /*default:
                log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
                break;*/
        }
    }
}


void atender_dispatch_cpu() 
{
    t_buffer* buffer;
    int cod_op;
    cliente_kernel_dispatch = esperar_cliente(socket_kernel_cpu_dispatch);
    log_info(logger_kernel, "Se conectó DISPATCH");

    while(1) 
    {
        cod_op = recibir_operacion(socket_kernel_cpu_dispatch);
        switch (cod_op) 
        {
            case MENSAJE:
                //recibir_mensaje(socket_kernel_cpu_dispatch, logger_kernel);
                break;
            case HANDSHAKE_CPU_KERNEL_D:
            buffer = recibiendo_super_paquete(socket_kernel_cpu_dispatch);
            int identificador2 = recibir_int_del_buffer(buffer);
            break;
            /*case -1:
                log_error(logger_kernel, "KERNEL DISPATCH se desconecto. Terminando servidor");
                pthread_exit(NULL);*/
            /*default:
                log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
                break;*/
        }
    }
}

void atender_interrupcion_cpu() 
{
    t_buffer* buffer;
    int cod_op;
    cliente_kernel_interrupt = esperar_cliente(socket_kernel_cpu_interrupt);
    log_info(logger_kernel, "Se conectó INTERRUPT");

    while(1) 
    {
        cod_op = recibir_operacion(socket_kernel_cpu_interrupt);
        switch (cod_op) 
        {
            case MENSAJE:
                //recibir_mensaje(socket_kernel_cpu_interrupt, logger_kernel);
                break;
            case HANDSHAKE_CPU_KERNEL_I:
            buffer = recibiendo_super_paquete(socket_kernel_cpu_interrupt);
            int identificador = recibir_int_del_buffer(buffer);
            break;
            /*case -1:
                log_error(logger_kernel, "KERNEL INTERRUPT se desconecto. Terminando servidor");
                pthread_exit(NULL);*/
            /*default:
                log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
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
                //recibir_mensaje(socket_kernel_io, logger_kernel);
                break;
            case -1:
                log_error(logger_kernel, "KERNEL se desconecto. Terminando servidor");
                pthread_exit(NULL);
            default:
                log_warning(logger_kernel, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}