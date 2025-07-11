#include "configs.h"


t_config* iniciar_config(char* archivo_config)
{
	t_config* nuevo_config = config_create(archivo_config);
	if(nuevo_config == NULL)
    {
		//printf("No se puede crear la config\n");
		exit(EXIT_FAILURE);
	}
    
	return nuevo_config;
}

t_log* iniciar_logger(char* pathLogger, char* nombreLogger,t_log_level logLevel)
{
	t_log* nuevo_logger;
    nuevo_logger = log_create(pathLogger, nombreLogger, 1,logLevel);
	
    log_info(nuevo_logger,"%s iniciado",nombreLogger);
    
    if(nuevo_logger == NULL)
    {
		//printf("No se puede crear el logger\n");
		exit(EXIT_FAILURE);
	}

	return nuevo_logger;
}

