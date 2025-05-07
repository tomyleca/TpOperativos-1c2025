#include <memoria.h>

int main(int argc, char* argv[]) {
    saludar("memoria");

    // INICIO Y LEO CONFIG
    config_memoria = iniciar_config("memoria.config");
    leerConfigMemoria(config_memoria);

    // INICIO LOGGER
    logger_memoria = iniciar_logger("memoriaLogger.log", "memoriaLogger", log_level);

   inicializar_estructuras_memoria();

    int fd_escucha_servidor = iniciar_servidor(logger_memoria, puerto_escucha);
    log_info(logger_memoria, "Servidor listo para recibir clientes");
    server_escucha(fd_escucha_servidor, logger_memoria);
    // EL SERVER_ESCUCHA ES QUIEN CREA EL SERVIDOR MULTIHILOS, QUE RECIBE VARIOS HILOS A LA VEZ.

    return EXIT_SUCCESS;
}
