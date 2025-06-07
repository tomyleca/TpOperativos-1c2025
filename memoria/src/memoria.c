#include <memoria.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
    // INICIO Y LEO CONFIG
    config_memoria = iniciar_config("memoria.config");
    leerConfigMemoria(config_memoria); 
    inicializar_memoria();// inicializa la memoria con los datos de memoria.config
    //INICIO LOGGER 
    logger_memoria = iniciar_logger("memoriaLogger.log","memoriaLogger",log_level);
    inicializar_swap();    // inicializa el sistema de SWAP
    diccionarioProcesos = crearDiccionarioConSemaforos();
    log_info(logger_memoria, "Servidor listo para recibir clientes"); 
    fd_escucha_servidor = iniciar_servidor(logger_memoria, puerto_escucha);
    mkdir(dump_path, 0777);
    lista_contextos = list_create();

    pthread_create(&hilo_memoria,NULL, (void*)server_escucha,&fd_escucha_servidor);

    pthread_join(hilo_memoria,NULL);

    //! ACA SE LIBRERA TODA LA MEMORIA OJO 
    cerrar_swap();
    free(memoria_real);
    free(bitmap_frames);
    list_destroy_and_destroy_elements(lista_contextos, (void*)free);
    log_destroy(logger_memoria);
    config_destroy(config_memoria);
 
    return EXIT_SUCCESS;
}
