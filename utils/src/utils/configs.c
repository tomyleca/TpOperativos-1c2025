#include "configs.h"

t_config* iniciar_config(char* archivo_config)
{
	t_config* nuevo_config = config_create(archivo_config);
	if(nuevo_config == NULL)
    {
		printf("No se puede crear la config\n");
		exit(EXIT_FAILURE);
	}
    
	return nuevo_config;
}

t_log* iniciar_logger(char* nombre_logger, char* nombre_proceso)
{
	t_log* nuevo_logger;
    nuevo_logger = log_create(nombre_logger, nombre_proceso, 1, LOG_LEVEL_INFO);
	
    log_info(nuevo_logger, "%s iniciado", nombre_proceso);
    
    if(nuevo_logger == NULL)
    {
		printf("No se puede crear el logger\n");
		exit(EXIT_FAILURE);
	}

	return nuevo_logger;
}

