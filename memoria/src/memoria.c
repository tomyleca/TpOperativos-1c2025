#include <memoria.h>

int main(int argc, char* argv[]) {
  
    saludar("memoria");
    // INICIO Y LEO CONFIG
    config_memoria = iniciar_config("memoria.config");
    leerConfigMemoria(config_memoria); 

    //INICIO LOGGER 
    logger_memoria = iniciar_logger("memoriaLogger.log","Logger de memoria",log_level);

    //INICIO MEMORIA
    inicializar_memoria();


    diccionarioProcesos = crearDiccionarioConSemaforos();
	log_info(logger_memoria, "Servidor listo para recibir clientes"); 
    fd_escucha_servidor = iniciar_servidor(logger_memoria, puerto_escucha);

	pthread_create(&hilo_memoria,NULL, (void*)server_escucha,&fd_escucha_servidor);

    pthread_join(hilo_memoria,NULL);

   //LIBRERA TODA LA MEMORIA 
    free(memoria_real);
    free(bitmap_frames);
 
    return EXIT_SUCCESS;

}

