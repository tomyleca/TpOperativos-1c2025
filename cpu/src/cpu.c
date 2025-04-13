#include <cpu.h>

int main(int argc, char* argv[]) {
    saludar("cpu");

    //INICIO Y LEO CONFIG
    t_config* config_cpu = iniciar_config("cpu.config");
    leerConfigCpu(config_cpu);
    
    //INICIO LOGGER
    logger_cpu = iniciar_logger("cpuLogger.log","cpuLogger",log_level);

    //INICIO CONEXION CON KERNEL
    int socket_cpu_kernel_dispatch = crear_conexion(logger_cpu, ip_kernel, puerto_kernel_dispatch);

    if(socket_cpu_kernel_dispatch == -1) {
        log_error(logger_cpu, "No se pudo conectar con Kernel en DISPATCH");
        exit(1);
    }
    log_info(logger_cpu, "Conexión establecida con el Kernel en DISPATCH");

    int socket_cpu_kernel_interrupt = crear_conexion(logger_cpu, ip_kernel, puerto_kernel_interrupt);

    if(socket_cpu_kernel_interrupt == -1) {
        log_error(logger_cpu, "No se pudo conectar con Kernel en INTERRUPT");
        exit(1);
    }
    log_info(logger_cpu, "Conexión establecida con el Kernel en INTERRUPT");

    //CIERRO
    log_info(logger_cpu, "cerrando conexión");
    liberar_conexion(socket_cpu_kernel_dispatch);
    liberar_conexion(socket_cpu_kernel_interrupt);

    return 0;
}

void leerConfigCpu(t_config* config_cpu) {
    ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config_cpu, "PUERTO_MEMORIA");
    ip_kernel = config_get_string_value(config_cpu, "IP_KERNEL");
    puerto_kernel_dispatch = config_get_int_value(config_cpu, "PUERTO_KERNEL_DISPATCH");
    puerto_kernel_interrupt = config_get_int_value(config_cpu, "PUERTO_KERNEL_INTERRUPT");
    entradas_tlb = config_get_int_value(config_cpu, "ENTRADAS_TLB");
    reemplazo_tlb = config_get_string_value(config_cpu, "REEMPLAZO_TLB");
    entradas_cache = config_get_int_value(config_cpu, "ENTRADAS_CACHE");
    reemplazo_cache = config_get_string_value(config_cpu, "REEMPLAZO_CACHE");
    retardo_cache = config_get_int_value(config_cpu, "RETARDO_CACHE");
    log_level = log_level_from_string(config_get_string_value(config_cpu, "LOG_LEVEL"));
    
}
