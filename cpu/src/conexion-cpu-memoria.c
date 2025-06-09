#include "conexion-cpu-memoria.h"

void crear_handshake_cpu_kernel_dispatch(int conexion_cpu_kernel)
{
    t_paquete* paquete = crear_super_paquete(HANDSHAKE_CPU_KERNEL_D);
    cargar_string_al_super_paquete(paquete, identificador_cpu);
    enviar_paquete(paquete,conexion_cpu_kernel);
    eliminar_paquete(paquete);
}

void crear_handshake_cpu_kernel_interrupt(int conexion_cpu_kernel)
{
    t_paquete* paquete = crear_super_paquete(HANDSHAKE_CPU_KERNEL_I);
    cargar_string_al_super_paquete(paquete, identificador_cpu);
    enviar_paquete(paquete,conexion_cpu_kernel);
    eliminar_paquete(paquete);
}

pthread_t escuchar_interrupcion_kernel()
{
    pthread_t hilo_interrupcion_kernel;
    pthread_create(&hilo_interrupcion_kernel, NULL, (void*) atender_interrupcion_kernel, NULL);
    return hilo_interrupcion_kernel;
}

pthread_t escuchar_memoria()
{
    pthread_t hilo_memoria;
    pthread_create(&hilo_memoria, NULL, (void*) atender_memoria, NULL);
    return hilo_memoria;
}

pthread_t escuchar_kernel()
{
    pthread_t hilo_dispatch_kernel;
    pthread_create(&hilo_dispatch_kernel, NULL, (void*) atender_dispatch_kernel, NULL);
    return hilo_dispatch_kernel;
}

void atender_memoria()
{ 
    t_buffer* buffer;
    int cod_op;
    int direccion_fisica;

    while(1) 
    {
        cod_op = recibir_operacion(socket_cpu_memoria);
        switch (cod_op) 
        {
            case RECIBIR_TAMANO_PAG:
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                tamanio_pagina = recibir_int_del_buffer(buffer);
                cant_niveles = recibir_int_del_buffer(buffer);
                cant_entradas_tabla = recibir_int_del_buffer(buffer);
                break;
            case CPU_RECIBE_INSTRUCCION_MEMORIA: 
                //ACA LLEGA LA SOLICITUD DE LA INSTRUCCION DE MEMORIA
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                contexto->pid = recibir_uint32_t_del_buffer(buffer);
                instruccion_recibida = recibir_string_del_buffer(buffer); // instruccion_recibida se usa en instruccion.c
                sem_post(&sem_hay_instruccion);
                //free(buffer);
                break;

            case CPU_RECIBE_OK_DE_LECTURA:
               buffer = recibiendo_super_paquete(socket_cpu_memoria);
                contexto->pid = recibir_int_del_buffer(buffer);
                direccion_fisica = recibir_int_del_buffer(buffer); // POR EL MOMENTO TRATAMOS A LA DIRECCION FISICA COMO INT 
                //valor_leido_de_memoria_32 = recibir_int_del_buffer(buffer);
                free(buffer);
                break;

            case CPU_RECIBE_OK_DE_ESCRITURA:
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                contexto->pid = recibir_int_del_buffer(buffer);
                direccion_fisica = recibir_int_del_buffer(buffer);
                            
                free(buffer); 
                break;
            case -1:
                log_error(logger_cpu, "MEMORIA se desconecto. Terminando servidor");
                shutdown(socket_cpu_memoria, SHUT_RDWR);
                close(socket_cpu_memoria);
                pthread_exit(NULL);
            default:
                log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}

void atender_interrupcion_kernel() 
{
    t_buffer* buffer;
    int cod_op;
    
    while(1) 
    {
        printf("ANTES DE RECIBIR OTRO PROCESO O MENSAJE en INTERRUPT");
        cod_op = recibir_operacion(socket_cpu_kernel_interrupt);
        switch (cod_op) 
        {
            case MENSAJE:
                //recibir_mensaje(socket_cpu_kernel_interrupt, logger_cpu);
                break;    
            case INTERRUPCION:
                
                log_info(logger_cpu, " ## Llega interrupción al puerto Interrupt.");
                
                pthread_mutex_lock(&mutex_motivo_interrupcion);
                flag_interrupcion = true;
                printf("Adentro de lmutex interrup \n");
                motivo_interrupcion = INTERRUPCION;
                pthread_mutex_unlock(&mutex_motivo_interrupcion);

                enviarOK(socket_cpu_kernel_interrupt);
                


                free(buffer);
                break;
            case -1:
                log_error(logger_cpu, "KERNEL INTERRUPT se desconecto. Terminando servidor");
                shutdown(socket_cpu_kernel_interrupt, SHUT_RDWR);
                close(socket_cpu_kernel_interrupt);
                pthread_exit(NULL);
            default:
                log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}


void atender_dispatch_kernel() 
{
    t_buffer* buffer;
    t_paquete* paquete;
    int cod_op;

    while(1) 
    {
        printf("ANTES DE RECIBIR OTRO PROCESO O MENSAJE en dispatch\n");
        cod_op = recibir_operacion(socket_cpu_kernel_dispatch);
        switch (cod_op) 
        {
            case MENSAJE:
                /*recibir_mensaje(socket_cpu_kernel_dispatch, logger_cpu);
                int valor = 5;
                crear_paquete()
                enviar_paquete(paquete, socket_cpu_kernel_dispatch); 
                eliminar_paquete(paquete);*/
                break;
                case PID_KERNEL_A_CPU:
                printf("ANTES DE RECIBIR OTRO PROCESO\n");
                
                contexto = malloc(sizeof(t_contexto_cpu));
                buffer = recibiendo_super_paquete(socket_cpu_kernel_dispatch);
                contexto->pid = recibir_uint32_t_del_buffer(buffer);
                contexto->registros.PC = recibir_uint32_t_del_buffer(buffer);
                enviarOK(socket_cpu_kernel_dispatch);
                // ACA HAY QUE SOLICITAR A MEMORIA LA PRIMER INSTRUCCION CON EL PID RECIBIMOS DE KERNEL
                ciclo_instruccion(socket_cpu_memoria); 
                free(buffer);
                break;
                
            case -1:
                log_error(logger_cpu, "KERNEL DISPATCH se desconecto. Terminando servidor");
                shutdown(socket_cpu_kernel_dispatch, SHUT_RDWR);
                close(socket_cpu_kernel_dispatch);
                pthread_exit(NULL);
                break;
            default:
                log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata. Estoy en kernel dispatch");
                break;
        }
    } 
}

