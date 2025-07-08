#include "conexion-memoria.h"

void server_escucha(int* fd_escucha_servidor)
{

    log_info(logger_memoria, "## Kernel Conectado - FD del socket: <%d>", *fd_escucha_servidor);


    while (1) {
        // Espera a un cliente en el bucle principal
        int fd_conexion_2 = *fd_escucha_servidor;
        int fd_conexion = esperar_cliente(fd_conexion_2);
        log_info(logger_memoria, "\n");
        log_info(logger_memoria, "## Cliente conectado - FD del socket: <%d>", fd_conexion);

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
    Proceso* p;
    t_paquete* paquete;
    int cliente_fd = *fd_conexion;
    int direccion_fisica;
    int tamanio;
    char* valor_Leido;
    char* valor_registro;
    char* clave;


    printf("---------------------------------------------\n");
    printf("Atendiendo cliente con fd: %d\n", cliente_fd);

    while (1) {
        int cod_op = recibir_operacion(cliente_fd);
        switch (cod_op) {
            // ! KERNEL
            case MENSAJE:
                //recibir_mensaje(cliente_fd);
                break;

            case SOLICITUD_ESTRUCTURA_MEMORIA:
                usleep(retardo_memoria * 1000);
                printf("SOLICITUD ESTRUCTURA MEMORIA------------------------------------------------------------------\n");
                paquete = crear_super_paquete(RESPUESTA_ESTRUCTURA_MEMORIA);
                cargar_int_al_super_paquete(paquete, TAM_PAGINA);
                cargar_int_al_super_paquete(paquete, CANTIDAD_NIVELES);
                cargar_int_al_super_paquete(paquete, ENTRADAS_POR_TABLA);
                enviar_paquete(paquete, cliente_fd);
                eliminar_paquete(paquete);
                break;
        
            case GUARDAR_PROCESO_EN_MEMORIA:
                usleep(retardo_memoria * 1000);
                printf("GUARDAR PROCESO EN MEMORIA------------------------------------------------------------------\n");
                t_info_kernel datos_kernel; 
                unBuffer = recibiendo_super_paquete(cliente_fd);
                datos_kernel.pid = recibir_uint32_t_del_buffer(unBuffer);
                datos_kernel.tamanio_proceso = recibir_uint32_t_del_buffer(unBuffer); 
                datos_kernel.archivo_pseudocodigo = recibir_string_del_buffer(unBuffer);
                printf("PID LLEGADO DE KERNEL: %d -TAMAÑO: %d - ARCHIVO PSEUDOCODIGO: %s\n", datos_kernel.pid, datos_kernel.tamanio_proceso, datos_kernel.archivo_pseudocodigo);
      
                if(guardarProcesoYReservar(datos_kernel.pid,datos_kernel.tamanio_proceso,datos_kernel.archivo_pseudocodigo) == -1)
                    enviarOpCode(cliente_fd,NO_HAY_MEMORIA);
                else 
                    enviarOpCode(cliente_fd,OK);
                limpiarBuffer(unBuffer);
                break;   
             
             case CPU_PIDE_INSTRUCCION_A_MEMORIA: //PARA INICIAR DECODE ESTO!!
                usleep(retardo_memoria * 1000);
                unBuffer = recibiendo_super_paquete(cliente_fd);
                buscar_y_mandar_instruccion(unBuffer,cliente_fd);
                limpiarBuffer(unBuffer);
                break;

            case SOLICITUD_TABLA:
                usleep(retardo_memoria * 1000);
                enviarOpCode(cliente_fd,RESPUESTA_SOLICITUD_TABLA); //SIMULO BUSCAR LA ENTRADA EN EL NIVEL, NO ES NECESARIO
                break;


            case SOLICITUD_FRAME:
                usleep(retardo_memoria * 1000);
                unBuffer = recibiendo_super_paquete(cliente_fd);
                uint32_t pid = recibir_uint32_t_del_buffer(unBuffer);
                clave = pasarUnsignedAChar(pid);
                Proceso* proceso = leerDeDiccionario(diccionarioProcesos,clave);
                free(clave);
                TablaPagina* tabla_nivel_X = proceso->tabla_raiz;
                int marco;
                
                for(int nivel = 1;nivel <= CANTIDAD_NIVELES; nivel++)
                {
                    entrada_nivel_X = recibir_int_del_buffer(unBuffer);
                    
                    
                    if(nivel < CANTIDAD_NIVELES)
                    {
                        tabla_nivel_X = tabla_nivel_X->entradas[entrada_nivel_X]; 
                    }
                    else
                    {
                        marco = tabla_nivel_X->frames[entrada_nivel_X];
                        
                            //TODO manejo de errores en cpu, seg fault
                    }
                }

                t_paquete* paquete = crear_super_paquete(RESPUESTA_SOLICITUD_FRAME);
                cargar_int_al_super_paquete(paquete,marco);
                enviar_paquete(paquete,cliente_fd);
                eliminar_paquete(paquete);
                limpiarBuffer(unBuffer);
                break;

            case CPU_PIDE_LEER_MEMORIA:
                usleep(retardo_memoria* 1000);
                printf("CPU_PIDE_LEER_MEMORIA------------------------------------------------------------------\n");
                unBuffer = recibiendo_super_paquete(cliente_fd);
                pid = recibir_int_del_buffer(unBuffer);
                direccion_fisica = recibir_int_del_buffer(unBuffer);
                tamanio = recibir_int_del_buffer(unBuffer);
                limpiarBuffer(unBuffer);
                printf("PID: %d - DIRECCION FISICA: %d - TAMAÑO: %d\n", pid, direccion_fisica, tamanio);
                p = leerDeDiccionario(diccionarioProcesos,pasarUnsignedAChar(pid));
                valor_Leido = leer_memoria(p,direccion_fisica,tamanio);
                printf("VALOR LEIDO: %s\n", valor_Leido);
                // Respuesta a CPU
                paquete = crear_super_paquete(RESPUESTA_VALOR_LEIDO_CPU);
                cargar_string_al_super_paquete(paquete, valor_Leido);  
                enviar_paquete(paquete, cliente_fd);
                eliminar_paquete(paquete);
                free(valor_Leido);
                break;

            case CPU_PIDE_ESCRIBIR_MEMORIA:
                usleep(retardo_memoria * 1000);
                printf("CPU_PIDE_ESCRIBIR_MEMORIA------------------------------------------------------------------\n");
                unBuffer = recibiendo_super_paquete(cliente_fd);
                pid = recibir_int_del_buffer(unBuffer);
                direccion_fisica = recibir_int_del_buffer(unBuffer);
                valor_registro = recibir_string_del_buffer(unBuffer);
                limpiarBuffer(unBuffer);
                printf("PID: %d - DIRECCION FISICA: %d - VALOR: %s\n", pid, direccion_fisica, valor_registro);
                
                p = leerDeDiccionario(diccionarioProcesos,pasarUnsignedAChar(pid));

                if (escribir_memoria(p, direccion_fisica, valor_registro) == 1) {
                    enviarOpCode(cliente_fd,CPU_RECIBE_OK_DE_ESCRITURA);
                } else {
                    //TODO: que hacer si falla la escritura?
                }
                break;
            
             case CPU_SOLICITA_LEER_PAGINA_COMPLETA:
                usleep(retardo_memoria * 1000);
                unBuffer = recibiendo_super_paquete(cliente_fd);
                pid = recibir_int_del_buffer(unBuffer);
                direccion_fisica = recibir_int_del_buffer(unBuffer);
                tamanio = recibir_int_del_buffer(unBuffer);
                limpiarBuffer(unBuffer);
                printf("PID: %d - DIRECCION FISICA: %d - TAMAÑO: %d\n", pid, direccion_fisica, tamanio);
                clave = pasarUnsignedAChar(pid);
                p = leerDeDiccionario(diccionarioProcesos,clave);
                free(clave);
                valor_Leido = leer_memoria(p,direccion_fisica,tamanio);
                
                // Respuesta a CPU
                paquete = crear_super_paquete(RESPUESTA_PAGINA_COMPLETA_CPU);
                cargar_string_al_super_paquete(paquete, valor_Leido); 
                enviar_paquete(paquete, cliente_fd);
                eliminar_paquete(paquete);
                free(valor_Leido);
                break;

            case CPU_SOLICITA_ESCRIBIR_PAGINA_COMPLETA:
                usleep(retardo_memoria * 1000);
                unBuffer = recibiendo_super_paquete(cliente_fd);
                pid = recibir_int_del_buffer(unBuffer);
                direccion_fisica = recibir_int_del_buffer(unBuffer);
                char* valor_a_escribir = recibir_string_del_buffer(unBuffer);
                limpiarBuffer(unBuffer);
                clave = pasarUnsignedAChar(pid);
                p = leerDeDiccionario(diccionarioProcesos,clave);
                free(clave);
                if(p!=NULL) //Si es NULL quiere decir que el proceso ya finalizo, y no hace falta escribir la pagina en memoria
                    escribir_memoria(p,direccion_fisica,valor_a_escribir);

                

                enviarOpCode(cliente_fd,RESPUESTA_ESCRITURA_PAGINA_COMPLETA);
                free(valor_a_escribir);
                break;

            case SWAP_SUSPENDER_PROCESO:
                usleep(retardo_swap * 1000);
                printf("SWAP_SUSPENDER_PROCESO ------------------------------------------------------------------\n");
            
                unBuffer = recibiendo_super_paquete(cliente_fd);
                pid = recibir_uint32_t_del_buffer(unBuffer);
                
                
            
                Proceso* p_suspend = leerDeDiccionario(diccionarioProcesos, pasarUnsignedAChar(pid));


                limpiarBuffer(unBuffer);
                if (!p_suspend) {
                    log_error(logger_memoria, "No se encontró el proceso PID %d para suspender.", pid);
                    paquete = crear_super_paquete(SWAP_ERROR);
                }
            
               if ( suspender_proceso(p_suspend) == -1) {
                    log_error(logger_memoria, "Error al suspender el proceso PID <%d>.", pid);
                    paquete = crear_super_paquete(SWAP_ERROR);
                } else{
                    log_info(logger_memoria, "Proceso PID %d suspendido correctamente.", pid);
                    paquete = crear_super_paquete(SWAP_OK);
                }
     
                
                enviar_paquete(paquete, cliente_fd);
                eliminar_paquete(paquete);
                break;
            
            case SWAP_RESTAURAR_PROCESO:
                usleep(retardo_swap * 1000);
                printf("SWAP_RESTAURAR_PROCESO ------------------------------------------------------------------\n");
            
                unBuffer = recibiendo_super_paquete(cliente_fd);
                pid = recibir_uint32_t_del_buffer(unBuffer);
                free(unBuffer);
                printf("PID: %d - DIRECCION FISICA: %d\n", pid, direccion_fisica);
            
                Proceso* p_restaurar = leerDeDiccionario(diccionarioProcesos, pasarUnsignedAChar(pid));

                if (!p_restaurar) {
                    log_error(logger_memoria, "No se encontró el proceso PID <%d> para restaurar.", pid);
                    paquete = crear_super_paquete(SWAP_ERROR);
                }
            
                if (   restaurar_proceso(p_restaurar) == -1) {
                    paquete = crear_super_paquete(SWAP_ERROR);
                } else{
                    paquete = crear_super_paquete(SWAP_OK);
                }
        
                
                enviar_paquete(paquete, cliente_fd);
                eliminar_paquete(paquete);
                break;
            
            case DUMP_MEMORY:
                usleep(retardo_swap * 1000);
                printf("DUMP_MEMORY ------------------------------------------------------------------\n");
                unBuffer = recibiendo_super_paquete(cliente_fd);
                pid = recibir_uint32_t_del_buffer(unBuffer);
                limpiarBuffer(unBuffer);
                log_info(logger_memoria, "## PID: <%u> - Memory Dump solicitado", pid);
                
                printf("PID: %d\n", pid);

                if (realizar_dump_memoria(pid)) {
                    paquete = crear_super_paquete(DUMP_MEMORY_OK);
                    cargar_uint32_t_al_super_paquete(paquete, pid);
                    enviar_paquete(paquete, cliente_fd);
                } else {
                    paquete = crear_super_paquete(DUMP_MEMORY_ERROR);
                    cargar_uint32_t_al_super_paquete(paquete, pid);
                    enviar_paquete(paquete, cliente_fd);
                }
                eliminar_paquete(paquete);
            break;
            case FINALIZA_PROCESO:
                usleep(retardo_swap * 1000);
                unBuffer = recibiendo_super_paquete(cliente_fd);
                pid = recibir_uint32_t_del_buffer(unBuffer);
                
                destruir_proceso(pid);
                paquete = crear_super_paquete(EXIT_OK);
                enviar_paquete(paquete,cliente_fd);
                eliminar_paquete(paquete);

                limpiarBuffer(unBuffer);
                break;



            case -1:
                log_error(logger_memoria, "## El cliente se desconectó. Terminando servidor.\n");

                //shutdown(cliente_fd, SHUT_RDWR);
                close(cliente_fd);
                pthread_exit(NULL);  
                break;
         
            
            default:
                log_warning(logger_memoria, "Operación desconocida: <%d>", cod_op);
            break;
            }
        }
}

