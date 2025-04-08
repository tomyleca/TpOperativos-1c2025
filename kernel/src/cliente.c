#include <cliente.h>

int cliente_kernel_memoria(char* ip, char* puerto)
{
    // KERNEL ES CLIENTE DE MEMORIA

    socket_kernel_memoria = crear_conexion(logger_kernel, ip, puerto);

    if(socket_kernel_memoria == -1)
    {
        log_info(logger_kernel, "ERROR: No se pudo crear conexion KERNEL - MEMORIA");
        exit(1);
    }

    log_info(logger_kernel, "Conexion con el servidor MEMORIA creada");
    enviar_mensaje("Hola MEMORIA soy KERNEL", socket_kernel_memoria);
    return socket_kernel_memoria;
}

void leer_archivo_config(char* archivo_config)
{
  /* LEO EL ARCHIVO DE CONFIGURACION */
  config_kernel = iniciar_config(archivo_config);
  ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
  puerto_memoria = config_get_int_value(config_kernel, "PUERTO_MEMORIA");
  algoritmo_planificacion = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
  tiempo_suspension = config_get_int_value(config_kernel, "TIEMPO_SUSPENSION");
  log_level = config_get_string_value(config_kernel,"LOG_LEVEL");
  /*printf("Valores cargados desde el archivo de configuración:\n");
  printf("IP_MEMORIA: %s\n", ip_memoria);
  printf("PUERTO_MEMORIA: %d\n", puerto_memoria);
  printf("ALGORITMO_PLANIFICACION: %s\n", algoritmo_planificacion);
  printf("LOG_LEVEL: %s\n", log_level);*/

  // SE LEE BIEN EL ARCHIVO DE CONFIGURACION!!! 
}

