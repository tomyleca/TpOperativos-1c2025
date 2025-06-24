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
            case RESPUESTA_ESTRUCTURA_MEMORIA:
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                tamanio_pagina = recibir_int_del_buffer(buffer);
                cant_niveles = recibir_int_del_buffer(buffer);
                cant_entradas_tabla = recibir_int_del_buffer(buffer);
                limpiarBuffer(buffer);
                break;
            case CPU_RECIBE_INSTRUCCION_MEMORIA: 
                //ACA LLEGA LA SOLICITUD DE LA INSTRUCCION DE MEMORIA
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                instruccion_recibida = recibir_string_del_buffer(buffer); // instruccion_recibida se usa en instruccion.c
                log_info(logger_cpu,"# Llega instrucción de memoria: %s", instruccion_recibida);
                limpiarBuffer(buffer);
                sem_post(&sem_hay_instruccion);
                break;
            
            case RESPUESTA_SOLICITUD_TABLA:
                sem_post(&semLlegoPeticionTabla);
                break;

            case RESPUESTA_SOLICITUD_FRAME:
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                nro_marco = recibir_int_del_buffer(buffer);
                log_info(logger_cpu, "RESPUESTA_SOLICITUD_FRAME - PID: <%d> - Marco recibido: <%d>", contexto->pid, nro_marco);
                sem_post(&semLlegoPeticionMMU);
                break;

            case RESPUESTA_VALOR_LEIDO_CPU: 
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                valor_str_temp = recibir_string_del_buffer(buffer); // Recibe el valor como string
                // Copia el valor al buffer temporal global. Asegura espacio y terminador nulo.
                strncpy(valor_leido_memoria, valor_str_temp, sizeof(valor_leido_memoria) - 1);
                valor_leido_memoria[sizeof(valor_leido_memoria) - 1] = '\0';
                sem_post(&sem_valor_leido); 
                limpiarBuffer(buffer);
                break;


            case CPU_RECIBE_OK_DE_ESCRITURA:
                sem_post(&semOkEscritura);
                break;

            case RESPUESTA_PAGINA_COMPLETA_CPU: 
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                buffer_pagina_recibida = recibir_string_del_buffer(buffer);
                
                sem_post(&sem_pagina_recibida); // Señaliza que la página está disponible
                
                break;

            case -1:
                log_error(logger_cpu, "MEMORIA se desconecto. Terminando servidor");
                //shutdown(socket_cpu_memoria, SHUT_RDWR);
                close(socket_cpu_memoria);
                exit(1);
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
      
            case INTERRUPCION_SINCRONICA:
                
                log_info(logger_cpu, " ## Llega interrupción sincrónica al puerto Interrupt.");
                pthread_mutex_lock(&mutex_motivo_interrupcion);
                flag_interrupcion = true;
                printf("Adentro de lmutex interrup \n");
                motivo_interrupcion = INTERRUPCION_SINCRONICA;
                pthread_mutex_unlock(&mutex_motivo_interrupcion);
                enviarOK(socket_cpu_kernel_interrupt);
                
                
                break;

            case INTERRUPCION_ASINCRONICA:
                log_info(logger_cpu, " ## Llega interrupción asincrónica al puerto Interrupt.");
                
                pthread_mutex_lock(&mutex_motivo_interrupcion);
                flag_interrupcion = true;
                printf("Adentro de lmutex interrup \n");
                motivo_interrupcion = INTERRUPCION_ASINCRONICA;
                pthread_mutex_unlock(&mutex_motivo_interrupcion);
                enviarOK(socket_cpu_kernel_interrupt);
                break;
            case -1:
                log_error(logger_cpu, "KERNEL INTERRUPT se desconecto. Terminando servidor");
                //shutdown(socket_cpu_kernel_interrupt, SHUT_RDWR);
                close(socket_cpu_kernel_interrupt);
                exit(1);
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
            case 1:
                log_info(logger_cpu,"LLEGA OK");
                sem_post(&semOKDispatch);
                break;
                
                case PID_KERNEL_A_CPU:
                printf("ANTES DE RECIBIR OTRO PROCESO\n");
                contexto = malloc(sizeof(t_contexto_cpu));
                buffer = recibiendo_super_paquete(socket_cpu_kernel_dispatch);
                contexto->pid = recibir_uint32_t_del_buffer(buffer);
                sem_wait(&semMutexPC);
                contexto->registros.PC = recibir_uint32_t_del_buffer(buffer);
                sem_post(&semMutexPC);
                enviarOK(socket_cpu_kernel_dispatch);
                // ACA HAY QUE SOLICITAR A MEMORIA LA PRIMER INSTRUCCION CON EL PID RECIBIMOS DE KERNEL
                //Falta liberar contexto, pero como lo usamos en el ciclo y todo cpu, a lo mejor conviene liberarlo al final del todo.
                limpiarBuffer(buffer);
                sem_post(&semContextoCargado);
                break;
                
            case -1:
                log_error(logger_cpu, "KERNEL DISPATCH se desconecto. Terminando servidor");
                //shutdown(socket_cpu_kernel_dispatch, SHUT_RDWR);
                close(socket_cpu_kernel_dispatch);
                exit(1);
                //TODO revisar esto
                break;
            default:
                log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata. Estoy en kernel dispatch");
                break;
        }
    } 
}

