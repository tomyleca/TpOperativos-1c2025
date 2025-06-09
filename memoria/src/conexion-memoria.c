#include "conexion-memoria.h"

void server_escucha(int* fd_escucha_servidor)
{
    log_info(logger_memoria, "MEMORIA lista para recibir peticiones de KERNEL");

    while (1) {
        // Espera a un cliente en el bucle principal
        int fd_conexion_2 = *fd_escucha_servidor;
        int fd_conexion = esperar_cliente(fd_conexion_2);
        log_info(logger_memoria, "Cliente conectado y en espera.\n");
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
    t_paquete* paquete;
    int pid;
    int direccion_fisica;
    int tamanio;
    printf("Si llegue aca, es porque tengo un cliente de kernel o cpu");

    while (1) {
        int cod_op = recibir_operacion(cliente_fd); 
        switch (cod_op) {
            // ! KERNEL
            case MENSAJE:
                //recibir_mensaje(cliente_fd);
                break;
        
            case GUARDAR_PROCESO_EN_MEMORIA:
                t_info_kernel datos_kernel; 
                datos_kernel.pid = 0;
                datos_kernel.tamanio_proceso = 0;
                unBuffer = recibiendo_super_paquete(cliente_fd);
                datos_kernel.pid = recibir_uint32_t_del_buffer(unBuffer);
                datos_kernel.tamanio_proceso = recibir_uint32_t_del_buffer(unBuffer); 
                datos_kernel.archivo_pseudocodigo = recibir_string_del_buffer(unBuffer);
                printf("---------------------------------------------\n");
                printf("PID LLEGADO DE KERNEL %d\n", datos_kernel.pid);
                printf("Tamano LLEGADO DE KERNEL %d\n", datos_kernel.tamanio_proceso);
                mostrar_bitmap();
                if(guardarProcesoYReservar(datos_kernel.pid,datos_kernel.tamanio_proceso,datos_kernel.archivo_pseudocodigo) == -1)
                    enviar_paquete(crear_super_paquete(NO_HAY_MEMORIA),cliente_fd);
                else 
                    enviar_paquete(crear_super_paquete(OK),cliente_fd);
                free(unBuffer);
                
            break;   
             case CPU_PIDE_INSTRUCCION_A_MEMORIA: //PARA INICIAR DECODE ESTO!!
                usleep(retardo_memoria * 1000);
                paquete = crear_super_paquete(RECIBIR_TAMANO_PAG);
                cargar_int_al_super_paquete(paquete, TAM_PAGINA);
                cargar_int_al_super_paquete(paquete, CANTIDAD_NIVELES);
                cargar_int_al_super_paquete(paquete, ENTRADAS_POR_TABLA);
                enviar_paquete(paquete, cliente_fd);
                unBuffer = recibiendo_super_paquete(cliente_fd);
                buscar_y_mandar_instruccion(unBuffer,cliente_fd);
                eliminar_paquete(paquete);
            break;
            case CPU_PIDE_LEER_MEMORIA:
                usleep(retardo_memoria* 1000);
                printf("en CPU_PIDE_LEER_MEMORIA------------------------------------------------------------------\n");
                unBuffer = recibiendo_super_paquete(cliente_fd);
                pid = recibir_int_del_buffer(unBuffer);
                direccion_fisica = recibir_int_del_buffer(unBuffer);
                tamanio = recibir_int_del_buffer(unBuffer);
                printf("Direccion fisica en leer memoria: %d --------------------------------------------------------\n", direccion_fisica);
                free(unBuffer);
                //TODO ver que pasa aca con el tema de la direc fisica en memoria
                // Respuesta a CPU
                paquete = crear_super_paquete(CPU_RECIBE_OK_DE_LECTURA);
                cargar_int_al_super_paquete(paquete, pid);
                cargar_string_al_super_paquete(paquete, "OK");
                cargar_int_al_super_paquete(paquete, tamanio);
                enviar_paquete(paquete, cliente_fd);
                eliminar_paquete(paquete);
                break;

            case CPU_PIDE_ESCRIBIR_MEMORIA:
                usleep(retardo_memoria * 1000);
                printf("en CPU_PIDE_ESCRIBIR_MEMORIA------------------------------------------------------------------\n");
                unBuffer = recibiendo_super_paquete(cliente_fd);
                pid = recibir_int_del_buffer(unBuffer);
                direccion_fisica = recibir_int_del_buffer(unBuffer);
                uint32_t valor_registro = recibir_uint32_t_del_buffer(unBuffer);
                log_info(logger_memoria, "Valor del dato a ecribir en memoria: %d", valor_registro);
                free(unBuffer);
                
                // escribir_memoria(pid, direccion_fisica, valor_registro); //TODO hacer esto

                // Respuesta a CPU
                paquete = crear_super_paquete(CPU_RECIBE_OK_DE_ESCRITURA);
                cargar_int_al_super_paquete(paquete, pid);
                cargar_int_al_super_paquete(paquete, direccion_fisica);
                enviar_paquete(paquete, cliente_fd);
                eliminar_paquete(paquete);
                printf("despues de mandar el paquete a cpu de CPU_PIDE_ESCRIBIR_MEMORIA\n");
                break;
             
            case -1:
                log_error(logger_memoria, "El cliente se desconectó. Terminando servidor.");
                close(cliente_fd);
                pthread_exit(NULL);  
                break;
         
            
            default:
                log_warning(logger_memoria, "Operación desconocida.");
            break;
            }
        }
}
