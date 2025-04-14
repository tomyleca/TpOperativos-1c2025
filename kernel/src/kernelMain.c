#include <kernel.h>

int main(int argc, char* argv[]) {

    //INICIO Y LEO CONFIG
    t_config* config_kernel = iniciar_config("kernel.config");
    leerConfigKernel(config_kernel);
    
    //INICIO LOGGER
    logger_kernel = iniciar_logger("kernelLogger.log","kernelLogger",log_level);

    /****************CONEXION KERNEL CON IO*********************/

    //INICIO SERVIDOR KERNEL-IO
    socket_kernel_io = iniciar_servidor(logger_kernel, puerto_escucha_IO);
    log_info(logger_kernel, "Servidor  iniciado para IO");
    
    cliente_kernel = esperar_cliente(socket_kernel_io);
    log_info(logger_kernel, "Se conectó IO");

    //CIERRO
    log_info(logger_kernel, "Finalizando conexión");
    liberar_conexion(socket_kernel_io);


    /****************CONEXION KERNEL CON CPU*********************/

    //INICIO SERVIDOR KERNEL-CPU

    socket_kernel_cpu_dispatch = iniciar_servidor(logger_kernel, puerto_escucha_dispatch); 
    log_info(logger_kernel, "Servidor DISPATCH iniciado");

    socket_kernel_cpu_interrupt = iniciar_servidor(logger_kernel, puerto_escucha_interrupt); 
    log_info(logger_kernel, "Servidor INTERRUPT iniciado");
    
    cliente_kernel_dispatch = esperar_cliente(socket_kernel_cpu_dispatch);
    
    cliente_kernel_interrupt = esperar_cliente(socket_kernel_cpu_interrupt);


     /****************CONEXION KERNEL CON MEMORIA*********************/

    socket_kernel_memoria = crear_conexion(logger_kernel,ip_memoria,puerto_memoria);



    //CIERRO
    log_info(logger_kernel, "Finalizando conexión");
    liberar_conexion(socket_kernel_cpu_dispatch);
    liberar_conexion(socket_kernel_cpu_interrupt);
    liberar_conexion(socket_kernel_io);
    liberar_conexion(socket_kernel_memoria);
    return 0;
}

void leerConfigKernel(t_config* config_kernel) {
    
    ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config_kernel, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT");
    puerto_escucha_IO = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_IO");
    algoritmo_planificacion = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
    tiempo_suspension = config_get_int_value(config_kernel, "TIEMPO_SUSPENSION");
    log_level = log_level_from_string(config_get_string_value(config_kernel, "LOG_LEVEL"));
    
}

