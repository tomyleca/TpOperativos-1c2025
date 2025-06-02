#include <memoria.h>

int main(int argc, char* argv[]) {
  
    saludar("memoria");
    // INICIO Y LEO CONFIG
    config_memoria = iniciar_config("memoria.config");
    leerConfigMemoria(config_memoria); 
    inicializar_memoria();// inicializa la memoria con los datos de memoria.config
    diccionarioProcesos = crearDiccionarioConSemaforos();
     //INICIO LOGGER 
    logger_memoria = iniciar_logger("memoriaLogger.log","memoriaLogger",log_level);
	log_info(logger_memoria, "Servidor listo para recibir clientes"); 
    fd_escucha_servidor = iniciar_servidor(logger_memoria, puerto_escucha);

    //guardarProcesoYReservar(10,30,"albertito40");
    //guardarProcesoYReservar(16,65,"30");

	lista_contextos = list_create();
	pthread_create(&hilo_memoria,NULL, (void*)server_escucha,&fd_escucha_servidor);

 

    pthread_join(hilo_memoria,NULL);

   //! ACA SE LIBRERA TODA LA MEMORIA OJO 
    free(memoria_real);
     free(bitmap_frames);
 
     return EXIT_SUCCESS;

}
