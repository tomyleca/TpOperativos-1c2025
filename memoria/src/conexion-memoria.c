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


void server_escucha(int fd_escucha_servidor,t_log* memoria_logger)
{
    while (1) {
        // Espera a un cliente en el bucle principal
        int fd_conexion = esperar_cliente(fd_escucha_servidor);
        log_info(memoria_logger, "Cliente conectado y en espera.\n");
        if (fd_conexion != -1) {
            pthread_t hilo_conexion;
            // Reservamos memoria para pasar el socket conexion al hilo
            int* nueva_conexion = malloc(sizeof(int));
            *nueva_conexion = fd_conexion;
            //Responde al handshake del cliente que espera que se conecte.
			//responder_handshake(fd_conexion);
            // Crea un hilo para manejar la conexión del cliente
            pthread_create(&hilo_conexion, NULL,(void*) atender_cliente, nueva_conexion);
			pthread_detach(hilo_conexion);  // Detach para que no necesites un join más tarde
        }
    }
}

int atender_cliente(int *fd_conexion)
{    
    /*lista_particiones = inicializar_lista_particiones(PARTICIONES);*/
	t_buffer* unBuffer;
    int cliente_fd = *fd_conexion;

    while (1) {
        int cod_op = recibir_operacion(cliente_fd); 
        switch (cod_op) {
            // ! KERNEL
            case MENSAJE:
                //recibir_mensaje(cliente_fd);
                break;

            /*case SOLICITAR_MEMORIA_PROCESO:	
                //falta algoritmo para definir si hay espacio o no
                log_info(memoria_logger, "## Kernel Conectado - FD del socket: %ls", fd_conexion);

                unBuffer = recibiendo_super_paquete(cliente_fd);
                int rta_inicializacion_proceso = asignar_particion(lista_particiones, unBuffer);
                if (rta_inicializacion_proceso)
                {
                    responder_a_kernel_confirmacion_del_proceso_creado(cliente_fd);
                    free(unBuffer);
                    break;
                }
                else
                {
                    free(unBuffer);
                    break;
                }

            case SOLICITAR_MEMORIA_HILO:
                //falta algoritmo para definir si hay espacio o no
                unBuffer = recibiendo_super_paquete(cliente_fd);
                int rta_inicializacion_hilo = asignar_hilo_a_proceso(lista_particiones,unBuffer,PATH_INSTRUCCIONES);
                if (rta_inicializacion_hilo)
                {
                    responder_a_kernel_confirmacion_del_hilo_creado(cliente_fd);
                    free(unBuffer);
                    break;
                }
                else
                {
                    free(unBuffer);
                    break;
                }

            case FINALIZAR_HILO:
                //falta algoritmo para definir si hay espacio o no
                unBuffer = recibiendo_super_paquete(cliente_fd);
                int rta_finalizacion_hilo = liberar_hilo(unBuffer);
                if (rta_finalizacion_hilo)
                {
                    responder_a_kernel_confirmacion_del_hilo_finalizado(cliente_fd);
                    free(unBuffer);
                    break;
                }
                else
                {
                    free(unBuffer);
                    break;
                }               
            case MEMORY_DUMP:
                unBuffer = recibiendo_super_paquete(cliente_fd);
                fd_filesys = crear_conexion("fd_filesystem", IP_FILESYSTEM, PUERTO_FILESYSTEM);
                memory_dump(unBuffer, fd_filesys);
                break;*/
            case -1:
                //log_error(memoria_logger, "El cliente se desconectó. Terminando servidor.");
                return 0;  // Terminar el ciclo y finalizar el hilo
            
            default:
                //log_warning(memoria_logger, "Operación desconocida.");
                break;
            }
        }
}
