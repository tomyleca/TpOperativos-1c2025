#include "conexion-memoria.h"

void leerConfigMemoria(t_config* config_memoria) 
{
    puerto_escucha = config_get_int_value(config_memoria, "PUERTO_ESCUCHA");
    tam_memoria = config_get_int_value(config_memoria, "TAM_MEMORIA");
    tam_pagina = config_get_int_value(config_memoria, "TAM_PAGINA");
    entradas_por_tabla = config_get_int_value(config_memoria, "ENTRADAS_POR_TABLA");
    cant_niveles = config_get_int_value(config_memoria, "CANTIDAD_NIVELES");
    retardo_memoria = config_get_int_value(config_memoria, "RETARDO_MEMORIA");
    path_swapfile = config_get_string_value(config_memoria, "PATH_SWAPFILE");
    retardo_swap = config_get_int_value(config_memoria, "RETARDO_SWAP");
    log_level = log_level_from_string(config_get_string_value(config_memoria, "LOG_LEVEL"));
    dump_path = config_get_string_value(config_memoria, "DUMP_PATH"); 
}


void server_escucha(int fd_escucha_servidor, t_log* logger_memoria) 
{
    log_info(logger_memoria, "MEMORIA lista para recibir peticiones de KERNEL");
    
    while (1) 
    {        
        int socket_kernel_por_dudas = esperar_cliente(fd_escucha_servidor); 

        log_info(logger_memoria, "## Kernel Conectado - FD del socket: <%d>", socket_kernel_por_dudas);
        
        if (socket_kernel_por_dudas != -1) 
        {
            // Crear un nuevo hilo para atender la petición del cliente Kernel
            pthread_t hilo_atender_cliente;
            pthread_create(&hilo_atender_cliente, NULL, (void*) atender_cliente, &socket_kernel_por_dudas);            
            pthread_detach(hilo_atender_cliente); // No es necesario hacer join
        }
    }
}


int atender_cliente(int *fd_conexion)
{    
    t_buffer* unBuffer;
    int cliente_fd = *fd_conexion;
    t_paquete* paquete;
    int pid;
    printf("Si llegue aca, es porque tengo un cliente de kernel o cpu");
    while (1) {
        int cod_op = recibir_operacion(cliente_fd); 
        switch (cod_op) {
            case MENSAJE:
                //recibir_mensaje(cliente_fd);
                break;
        case CPU_PIDE_CONTEXTO: 
                //usleep(retardo_memoria * 1000); // Convertir milisegundos a microsegundos
                unBuffer = recibiendo_super_paquete(cliente_fd);
                pid = recibir_int_del_buffer(unBuffer);
                if(pid >= 0)
                {
                    nuevo_contexto_provisorio = buscar_contexto_por_pid(pid);                   
                    enviar_contexto(nuevo_contexto_provisorio, cliente_fd); 
                }
                else
                {
                    log_error(logger_memoria, "PID invalido");                   
                }
                free(unBuffer);
                free(nuevo_contexto_provisorio);
                break;
            case RECIBIR_PID_KERNEL:
            t_info_kernel datos_kernel; 
            datos_kernel.pid = 0;
            datos_kernel.tamanio_proceso = 0;
            unBuffer = recibiendo_super_paquete(cliente_fd);
            datos_kernel.pid = recibir_int_del_buffer(unBuffer);
            datos_kernel.tamanio_proceso = recibir_int_del_buffer(unBuffer); 
            datos_kernel.archivo_pseudocodigo = recibir_string_del_buffer(unBuffer);
            printf("---------------------------------------------\n");
            printf("PID LLEGADO DE KERNEL %d\n", datos_kernel.pid);
            nuevo_contexto = malloc(sizeof(t_contexto)); //ESTE MALLOC LO HAGO PORQUE NECESITO GUARDAR PUNTEROS A MI LISTA
            nuevo_contexto = buscar_contexto_por_pid(datos_kernel.pid);
            crear_pid(nuevo_contexto, datos_kernel);
            // Respuesta a KERNEL----- EL NUMERO DE TABLA DE PRIMER NIVEL !!
            //paquete = crear_super_paquete(RESPUESTA_KERNEL_TPN); //TPN TABLA DE PRIMER NIVEL --- a implementar todavia!! 
            //cargar_int_al_super_paquete(paquete, tabla_primer_nivel);
            //enviar_paquete(paquete, cliente_fd);
            free(unBuffer);
            free(datos_kernel.archivo_pseudocodigo);
            break;       
            case CPU_PIDE_INSTRUCCION_A_MEMORIA: //PARA INICIAR DECODE ESTO!!
                usleep(retardo_memoria * 1000);
                unBuffer = recibiendo_super_paquete(cliente_fd);
                buscar_y_mandar_instruccion(unBuffer,cliente_fd);
                free(unBuffer);
                break;
            case -1:
                log_error(logger_memoria, "El cliente se desconectó. Terminando servidor.");
                pthread_exit(NULL);
            
            default:
                //log_warning(memoria_logger, "Operación desconocida.");
                break;
            }
        }
}
