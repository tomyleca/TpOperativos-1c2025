#include <io.h>

int main(int argc, char* argv[]) {
    saludar("io");

    //INICIO Y LEO CONFIG
    config_io = iniciar_config("io.config");
    leerConfigIO(config_io);
    
    //INICIO LOGGER
    logger_io = iniciar_logger("ioLogger.log","ioLogger",log_level);

    //INICIO CONEXION CON KERNEL
    socket_io_kernel = crear_conexion(logger_io, ip_kernel, puerto_kernel);

    //CIERRO
    log_info(logger_io, "cerrando conexión");
    liberar_conexion(socket_io_kernel);

    return 0;
}

void leerConfigIO(t_config* config_io) 
{
    ip_kernel = config_get_string_value(config_io, "IP_KERNEL");
    puerto_kernel = config_get_int_value(config_io, "PUERTO_KERNEL");
    log_level = log_level_from_string(config_get_string_value(config_io, "LOG_LEVEL"));
}