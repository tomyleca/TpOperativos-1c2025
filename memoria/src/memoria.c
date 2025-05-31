#include <memoria.h>

int main(int argc, char* argv[]) {
  
    saludar("memoria");
    // INICIO Y LEO CONFIG
    config_memoria = iniciar_config("memoria.config");
    leerConfigMemoria(config_memoria);  
    inicializar_memoria();// inicializa la memoria con los datos de memoria.config
    mostrar_bitmap();//TODO: Borrar esto es testing
    Proceso *p=crear_proceso_y_reservar("30",30);
    dump_memory(p);
    free(p);
     //INICIO LOGGER
    logger_memoria = iniciar_logger("memoriaLogger.log","memoriaLogger",log_level);
	fd_escucha_servidor = iniciar_servidor(logger_memoria, puerto_escucha);
	log_info(logger_memoria, "Servidor listo para recibir clientes");
	lista_contextos = list_create();
	pthread_create(&hilo_memoria,NULL, (void*)server_escucha,&fd_escucha_servidor);

    //! ACA SE LIBRERA TODA LA MEMORIA OJO 
 
    free(memoria_real);
    free(bitmap_frames);

    pthread_join(hilo_memoria,NULL);

    return EXIT_SUCCESS;

}
