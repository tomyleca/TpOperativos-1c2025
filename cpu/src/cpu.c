#include <cpu.h>

int main(int argc, char* argv[]) {
    signal(SIGINT,liberarRecursos);
    
    saludar("cpu");

    //INICIO Y LEO CONFIG
    config_cpu = iniciar_config("cpu.config");
    leerConfigCpu(config_cpu);
    
    //INICIO LOGGER
    logger_cpu = iniciar_logger("cpuLogger.log","cpuLogger",log_level);

    inicializar_recursos();
    
    identificador_cpu = argv[1];  
    // INICIO HILOS
    inicializar_hilos(config_cpu);

    pthread_join(hilo_escuchar_memoria,NULL);
    pthread_join(hilo_escuchar_kernel,NULL);
    pthread_join(hilo_escuchar_kernel_interrupcion,NULL);


    return 0;
}

void leerConfigCpu(t_config* config_cpu) {
    ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    ip_kernel = config_get_string_value(config_cpu, "IP_KERNEL");
    puerto_kernel_dispatch = config_get_string_value(config_cpu, "PUERTO_KERNEL_DISPATCH");
    puerto_kernel_interrupt = config_get_string_value(config_cpu, "PUERTO_KERNEL_INTERRUPT");
    entradas_tlb = config_get_int_value(config_cpu, "ENTRADAS_TLB");
    reemplazo_tlb = config_get_string_value(config_cpu, "REEMPLAZO_TLB");
    entradas_cache = config_get_int_value(config_cpu, "ENTRADAS_CACHE");
    reemplazo_cache = config_get_string_value(config_cpu, "REEMPLAZO_CACHE");
    retardo_cache = config_get_int_value(config_cpu, "RETARDO_CACHE");
    log_level = log_level_from_string(config_get_string_value(config_cpu, "LOG_LEVEL"));
    
}

void inicializar_hilos(t_config* config_cpu)
{
    socket_cpu_memoria = crear_conexion(logger_cpu, ip_memoria, puerto_memoria);
    hilo_escuchar_memoria = escuchar_memoria();

    socket_cpu_kernel_dispatch = crear_conexion(logger_cpu, ip_kernel, puerto_kernel_dispatch);
    crear_handshake_cpu_kernel_dispatch(socket_cpu_kernel_dispatch);
    hilo_escuchar_kernel = escuchar_kernel();

    socket_cpu_kernel_interrupt = crear_conexion(logger_cpu,ip_kernel, puerto_kernel_interrupt); // CPU SERVIDOR DE KERNEL INTERRUPT -> envia a kernel el estado actual
    crear_handshake_cpu_kernel_interrupt(socket_cpu_kernel_interrupt);
	hilo_escuchar_kernel_interrupcion = escuchar_interrupcion_kernel();

    //ESTE SI LO DESCOMENTO, TIRA SEG FAULT PORQUE INICIA EL HILO EN DECODE, HABRIA QUE PONER SEMAFORO SUPONGO!!
}

void inicializar_recursos()
{
    // Inicializar semaforos
    sem_init(&sem_hay_instruccion, 0, 0);
    sem_init(&sem_interrupcion, 0, 1);

    iniciar_diccionario_instrucciones();
    
    pthread_mutex_init(&mutex_motivo_interrupcion, NULL);


    lista_tlb = list_create();
}




void liberarRecursos(int signal)
{
    if(signal != SIGINT)
        return;
    
    close(socket_cpu_kernel_dispatch);
    close(socket_cpu_kernel_interrupt);
    close(socket_cpu_memoria);
    exit(1);
}