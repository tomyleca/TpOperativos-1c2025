#include "conexion-cpu-memoria.h"

void crear_handshake_cpu_kernel_dispatch(int conexion_cpu_kernel)
{
    t_paquete* paquete = crear_super_paquete(HANDSHAKE_CPU_KERNEL_D);
    cargar_string_al_super_paquete(paquete, identificador_cpu);
    enviar_paquete(paquete,conexion_cpu_kernel);
    free(paquete);
}

void crear_handshake_cpu_kernel_interrupt(int conexion_cpu_kernel)
{
    t_paquete* paquete = crear_super_paquete(HANDSHAKE_CPU_KERNEL_I);
    cargar_string_al_super_paquete(paquete, identificador_cpu);
    enviar_paquete(paquete,conexion_cpu_kernel);
    free(paquete);
}

pthread_t crear_hilo_interpretar_instruccion()
{
    pthread_t hilo_decode;
    iniciar_diccionario_instrucciones();
    pthread_create(&hilo_decode, NULL, (void*)decode, NULL);
    return hilo_decode;
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
            case CPU_RECIBE_INSTRUCCION_MEMORIA: 
                //ACA LLEGA LA SOLICITUD DE LA INSTRUCCION DE MEMORIA
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                contexto->pid = recibir_int_del_buffer(buffer);
                instruccion_recibida = recibir_string_del_buffer(buffer); // instruccion_recibida se usa en instruccion.c
                sem_post(&sem_hay_instruccion);
                free(buffer);
                break;

            case CPU_RECIBE_CONTEXTO:  
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                contexto->pid = recibir_int_del_buffer(buffer);
                contexto->registros.PC = recibir_int_del_buffer(buffer); 
                cargar_registros(buffer);
                fetch(socket_cpu_memoria);
                //recibir_contexto(); 
                free(buffer);
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

            case CPU_RECIBE_OK_ACTUALIZAR_CONTEXTO: //MANDO PID CON LOS REGISTROS O PCB COMPLETO
               buffer = recibiendo_super_paquete(socket_cpu_memoria);
                contexto->pid = recibir_int_del_buffer(buffer);
                printf("Antes de semaforo de contexto\n");  
                sem_post(&sem_contexto);
                printf("Despues de semaforo de contexto\n");  
                free(buffer); 
                break;
            case -1:
                log_error(logger_cpu, "MEMORIA se desconecto. Terminando servidor");
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
            case -1:
                log_error(logger_cpu, "KERNEL INTERRUPT se desconecto. Terminando servidor");
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
                sem_wait(&sem_pid);
                
                printf("RECIBI OTRO PROCESO\n");
                sem_wait(&sem_contexto); // REVISAR EN QUE ALGORITMO SE USA
                printf("pase el semaforo OTRO PROCESO\n");
                contexto = malloc(sizeof(t_contexto_cpu));
                buffer = recibiendo_super_paquete(socket_cpu_kernel_dispatch);
                contexto->pid = recibir_int_del_buffer(buffer);
                contexto->registros.PC = recibir_int_del_buffer(buffer);
                // ACA HAY QUE SOLICITAR A MEMORIA EL CONTEXTO CON EL PID RECIBIMOS DE KERNEL
                solicitar_contexto_a_memoria(contexto); 
                free(buffer);
                break;
                
            case -1:
                log_error(logger_cpu, "KERNEL DISPATCH se desconecto. Terminando servidor");
                pthread_exit(NULL);
            default:
                log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata. Estoy en kernel dispatch");
                break;
        }
    } 
}

