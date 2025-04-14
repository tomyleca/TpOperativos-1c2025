#include <memoria.h>

int main(int argc, char* argv[]) {
    saludar("memoria");

    //INICIO Y LEO CONFIG
    config_memoria = iniciar_config("memoria.config");
    leerConfigMemoria(config_memoria);
    
    //INICIO LOGGER
    logger_memoria = iniciar_logger("memoriaLogger.log","memoriaLogger",log_level);

	int fd_escucha_servidor = iniciar_servidor(logger_memoria, puerto_escucha);

    if (fd_escucha_servidor == -1) {
        log_error(logger_memoria, "Error al iniciar el servidor");
        exit(1);
    }

	log_info(logger_memoria, "Servidor listo para recibir clientes");
	//fd_conexion_cpu = esperar_cliente(fd_escucha_servidor ,memoria_logger);
	
	server_escucha(fd_escucha_servidor, logger_memoria);
    //EL SERVER_ESCUCHA ES QUIEN CREA EL SERVIDOR MULTIHILOS, QUE RECIBE VARIOS HILOS A LA VEZ.
	
	return EXIT_SUCCESS;

    return 0;
}

void leerConfigMemoria(t_config* config_memoria) 
{
    puerto_escucha = config_get_int_value(config_memoria, "PUERTO_ESCUCHA");
    tam_memoria = config_get_int_value(config_memoria, "TAM_MEMORIA");
    tam_pagina = config_get_int_value(config_memoria, "TAM_PAGINA");
    entradas_por_tabla = config_get_int_value(config_memoria, "ENTRADAS_POR_TABLA");
    cant_niveles = config_get_int_value(config_memoria, "CANTIDAD_NIVELES");
    retardo_memoria config_get_int_value(config_memoria, "RETARDO_MEMORIA");
    path_swapfile = config_get_string_value(config_memoria, "PATH_SWAPFILE");
    retardo_swap = config_get_int_value(config_memoria, "RETARDO");
    log_level = log_level_from_string(config_get_string_value(config_memoria, "LOG_LEVEL"));
    dump_path = config_get_string_value(config_memoria, "DUMP_PATH"); 
}
