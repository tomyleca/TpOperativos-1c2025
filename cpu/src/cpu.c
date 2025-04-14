#include <cpu.h>

int main(int argc, char* argv[]) {
    saludar("cpu");

    //INICIO Y LEO CONFIG
    config_cpu = iniciar_config("cpu.config");
    leerConfigCpu(config_cpu);
    
    //INICIO LOGGER
    logger_cpu = iniciar_logger("cpuLogger.log","cpuLogger",log_level);

    //SERVIDOR CON KERNEL
    fd_cpu_kernel_dispatch = crear_conexion(logger_cpu, ip_kernel, puerto_kernel_dispatch);

    fd_cpu_kernel_interrupt = crear_conexion(logger_cpu,ip_kernel, puerto_kernel_interrupt);    //DSP LOS CAMBIO POR PTHREADS, los hice a parte 
    
    // INICIO CONEXION CON MEMORIA 
    socket_cpu_memoria = crear_conexion(logger_cpu, ip_memoria, puerto_memoria);


    //CIERRO
    log_info(logger_cpu, "Cerrando conexión");
    liberar_conexion(socket_cpu_kernel_dispatch);
    liberar_conexion(socket_cpu_kernel_interrupt);
    liberar_conexion(socket_cpu_memoria);

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
