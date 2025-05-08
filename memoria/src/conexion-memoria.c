#include "conexion-memoria.h"
typedef struct {
    int pid;               // ID del proceso
    int* paginas;          // Array de marcos asignados a las páginas del proceso
    int cantidad_paginas;  // Número de páginas asignadas
} Proceso;

Proceso* procesos = NULL;  // Lista de procesos
int cantidad_procesos = 0; // Número de procesos actuales

void server_escucha(int fd_escucha_servidor, t_log* logger_memoria) {
  while (1) {
    // Espera a un cliente en el bucle principal
    int fd_conexion = esperar_cliente(fd_escucha_servidor);
    log_info(logger_memoria, "Cliente conectado y en espera.\n");
    if (fd_conexion != -1) {
      pthread_t hilo_conexion;
      // Reservamos memoria para pasar el socket conexion al hilo
      int* nueva_conexion = malloc(sizeof(int));
      *nueva_conexion = fd_conexion;
      // Responde al handshake del cliente que espera que se conecte.
      // responder_handshake(fd_conexion);
      // Crea un hilo para manejar la conexión del cliente
      pthread_create(&hilo_conexion, NULL, (void*)atender_cliente, nueva_conexion);
      pthread_detach(hilo_conexion); // Detach para que no necesites un join más tarde
    }
  }
}

void asignar_pagina_a_proceso(int pid, int pagina, int marco) {
    for (int i = 0; i < cantidad_procesos; i++) {
        if (procesos[i].pid == pid) {
            if (pagina < procesos[i].cantidad_paginas) {
                procesos[i].paginas[pagina] = marco;
                log_info(logger_memoria, "Página %d del proceso PID %d asignada al marco %d", pagina, pid, marco);
                return;
            } else {
                log_error(logger_memoria, "Error: Página %d fuera de rango para el proceso PID %d", pagina, pid);
                return;
            }
        }
    }
    log_error(logger_memoria, "Error: Proceso PID %d no encontrado", pid);
}

bool crear_estructuras_proceso(int pid, int paginas_necesarias) {
       // Validar si hay espacio suficiente en memoria
       if (paginas_necesarias > (tam_memoria / tam_pagina)) {
        log_error(logger_memoria, "No hay suficientes páginas disponibles para el proceso PID %d", pid);
        return false;
    }

    // Registrar el nuevo proceso
    procesos = realloc(procesos, (cantidad_procesos + 1) * sizeof(Proceso));
    if (procesos == NULL) {
        log_error(logger_memoria, "Error al registrar el proceso PID %d", pid);
        return false;
    }

    procesos[cantidad_procesos].pid = pid;
    procesos[cantidad_procesos].cantidad_paginas = paginas_necesarias;
    procesos[cantidad_procesos].paginas = malloc(paginas_necesarias * sizeof(int));
    if (procesos[cantidad_procesos].paginas == NULL) {
        log_error(logger_memoria, "Error al asignar memoria para las páginas del proceso PID %d", pid);
        return false;
    }

    // Inicializar las páginas con -1 (sin asignar)
    for (int i = 0; i < paginas_necesarias; i++) {
        procesos[cantidad_procesos].paginas[i] = -1;
    }

    // Asignar marcos a las páginas
    for (int i = 0; i < paginas_necesarias; i++) {
        int marco = asignar_marco_libre();
        if (marco == -1) {
            log_error(logger_memoria, "No se pudo asignar un marco para el proceso PID %d", pid);
            return false;
        }
        asignar_pagina_a_proceso(pid, i, marco);
    }

    cantidad_procesos++;
    log_info(logger_memoria, "Estructuras de paginación creadas para el proceso PID %d", pid);
    return true;
}


void finalizar_proceso(int pid) {
    for (int i = 0; i < cantidad_procesos; i++) {
        if (procesos[i].pid == pid) {
            // Liberar los marcos asignados
            for (int j = 0; j < procesos[i].cantidad_paginas; j++) {
                if (procesos[i].paginas[j] != -1) {
                    liberar_marco(procesos[i].paginas[j]);
                }
            }
            free(procesos[i].paginas);

            // Eliminar el proceso de la lista
            for (int k = i; k < cantidad_procesos - 1; k++) {
                procesos[k] = procesos[k + 1];
            }
            cantidad_procesos--;
            procesos = realloc(procesos, cantidad_procesos * sizeof(Proceso));
            log_info(logger_memoria, "Proceso PID %d finalizado y recursos liberados", pid);
            return;
        }
    }
    log_error(logger_memoria, "Error: Proceso PID %d no encontrado", pid);
}

int atender_cliente(int *fd_conexion) {
  	t_buffer* unBuffer;
    int cliente_fd = *fd_conexion;
    free(fd_conexion); 
    
  while (1) {
    int cod_op = recibir_operacion(cliente_fd);
    switch (cod_op) {
    // ! KERNEL
    case MENSAJE:
      // recibir_mensaje(cliente_fd);
      break;

    case SOLICITAR_MEMORIA_PROCESO:

      unBuffer = recibiendo_super_paquete(cliente_fd);
      // Recibir datos del proceso
      int pid = recibir_int_del_buffer(unBuffer); // PID del proceso
      int tamanio = recibir_int_del_buffer(unBuffer); // Tamaño solicitado en bytes

      // Validar si hay espacio suficiente en memoria
      if (tamanio > tam_memoria) {
        log_error(logger_memoria,"No hay suficiente espacio en memoria para el proceso PID %d", pid);
        enviar_mensaje(cliente_fd, ERROR_MEMORIA_INSUFICIENTE); // Respuesta de error al Kernel
      } else {
        // Crear estructuras de paginación para el proceso
        int paginas_necesarias = (tamanio + tam_pagina - 1) / tam_pagina; // Redondeo hacia arriba
        if (crear_estructuras_proceso(pid, paginas_necesarias)) {
          log_info(logger_memoria, "Proceso PID %d creado con éxito", pid);
          enviar_mensaje(cliente_fd, OK); // Respuesta de éxito al Kernel
        } else {
          log_error(logger_memoria, "Error al crear estructuras para el proceso PID %d", pid);
          enviar_mensaje( cliente_fd, ERROR_CREACION_ESTRUCTURAS); // Respuesta de error al Kernel
        }
      }

      free(unBuffer);
      break;

      /* case SOLICITAR_MEMORIA_HILO:
           //falta algoritmo para definir si hay espacio o no
           unBuffer = recibiendo_super_paquete(cliente_fd);
           int rta_inicializacion_hilo =
       asignar_hilo_a_proceso(lista_particiones,unBuffer,PATH_INSTRUCCIONES); if
       (rta_inicializacion_hilo)
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
           fd_filesys = crear_conexion("fd_filesystem", IP_FILESYSTEM,
       PUERTO_FILESYSTEM); memory_dump(unBuffer, fd_filesys); break;*/
    case -1:
       log_error(logger_memoria, "El cliente se desconectó. Terminando servidor.");
      return 0; // Terminar el ciclo y finalizar el hilo

    default:
     log_warning(logger_memoria, "Operación desconocida.");
      break;
    }
  }
}
