#include <memoria.h>

int main(int argc, char* argv[]) {
  
    saludar("memoria");
    // INICIO Y LEO CONFIG
    config_memoria = iniciar_config("memoria.config");
    leerConfigMemoria(config_memoria);  
    inicializar_memoria();// inicializa la memoria con los datos de memoria.config
    tabla_raiz = crear_tabla_nivel(1);// aca se crean las tablas y frames 
    imprimir_tabla(tabla_raiz, 1, 0);//TODO: Borrar esto es testing
    mostrar_bitmap();//TODO: Borrar esto es testing

    //! ACA SIMULAMOS LLEGDA DE PAQUETE
    /*int cant_instrucciones = 0;
    char **instrucciones =
    leer_instrucciones("pseudocodigo.txt", &cant_instrucciones);
   
    //! ACA SIMULAMOS EJECUCION DE TODAS LAS INSTRUCCIONES 
    for (int i = 0; i < cant_instrucciones; i++) {
        interpretar_instruccion(instrucciones[i]);
        free(instrucciones[i]);
    }
    free(instrucciones);*/
  

     //INICIO LOGGER
    logger_memoria = iniciar_logger("memoriaLogger.log","memoriaLogger",log_level);
	fd_escucha_servidor = iniciar_servidor(logger_memoria, puerto_escucha);
	log_info(logger_memoria, "Servidor listo para recibir clientes");
	lista_contextos = list_create();
	pthread_create(&hilo_memoria,NULL, (void*)server_escucha,&fd_escucha_servidor);

    //! ACA SE LIBRERA TODA LA MEMORIA OJO 
    liberar_tabla(tabla_raiz);
    free(memoria_real);
    free(bitmap_frames);

    pthread_join(hilo_memoria,NULL);

    return EXIT_SUCCESS;

}
