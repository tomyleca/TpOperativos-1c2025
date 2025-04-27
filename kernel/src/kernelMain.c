#include <kernel.h>

int main(int argc, char* argv[]) {

    //INICIO Y LEO CONFIG
    t_config* config_kernel = iniciar_config("kernel.config");
    leerConfigKernel(config_kernel);
    
    //INICIO LOGGER
    logger_kernel = iniciar_logger("kernelLogger.log","kernelLogger",log_level);

    /****************CONEXION KERNEL CON IO*********************/

    //INICIO SERVIDOR KERNEL-IO
    int socket_kernel_io = iniciar_servidor(logger_kernel, puerto_escucha_IO); 

    inicializar_hilos(config_kernel);

    //CIERRO
    log_info(logger_kernel, "Finalizando conexión");
    pthread_join(hilo_escuchar_kernel, NULL);
    pthread_join(hilo_escuchar_kernel_interrupcion, NULL);
    pthread_detach(hilo_crear_kernel_memoria);

    INIT_PROC("afsfas",4);

    log_info(logger_kernel, "Servidor  iniciado para IO");

    int cliente_kernel = esperar_cliente(socket_kernel_io);
    log_info(logger_kernel, "Se conectó IO");

    //CIERRO
    log_info(logger_kernel, "Finalizando conexión");
    liberar_conexion(socket_kernel_io);


    /****************CONEXION KERNEL CON CPU*********************/

    //INICIO SERVIDOR KERNEL-CPU

    int socket_kernel_cpu_dispatch = iniciar_servidor(logger_kernel, puerto_escucha_dispatch); 

    if(socket_kernel_cpu_dispatch == -1) {
        log_error(logger_kernel, "Error al iniciar el servidor DISPATCH");
        exit(1);
    }
    log_info(logger_kernel, "Servidor DISPATCH iniciado");

    int socket_kernel_cpu_interrupt = iniciar_servidor(logger_kernel, puerto_escucha_interrupt); 

    if(socket_kernel_cpu_interrupt == -1) {
        log_error(logger_kernel, "Error al iniciar el servidor INTERRUPT");
        exit(1);
    }
    log_info(logger_kernel, "Servidor INTERRUPT iniciado");

    int cliente_kernel_dispatch = esperar_cliente(socket_kernel_cpu_dispatch);
    log_info(logger_kernel, "Se conectó cpu a dispatch");

    int cliente_kernel_interrupt = esperar_cliente(socket_kernel_cpu_interrupt);
    log_info(logger_kernel, "Se conectó cpu a interrupt");

    //CIERRO
    log_info(logger_kernel, "Finalizando conexión");
    liberar_conexion(socket_kernel_cpu_dispatch);
    liberar_conexion(socket_kernel_cpu_interrupt);

    cerrarConexiones();
    return 0;
}

void leerConfigKernel(t_config* config_kernel) {
    
    ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config_kernel, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT");
    puerto_escucha_IO = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_IO");
    algoritmo_planificacion = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
    algoritmo_cola_new = config_get_string_value(config_kernel, "ALGORITMO_COLA_NEW");
    alfa = config_get_int_value(config_kernel, "ALFA");
    tiempo_suspension = config_get_int_value(config_kernel, "TIEMPO_SUSPENSION");
    log_level = log_level_from_string(config_get_string_value(config_kernel, "LOG_LEVEL"));
    
}

void inicializar_hilos(t_config* config_kernel)
{   
    //TIENE MAS SENTIDO QUE PRIMERO CONECTE CON MEMORIA, Y DESPUES ESCUCHE PETICIONES DE CPU, NO? ENTONCES CAMBIE EL ORDEN!!
    socket_kernel_memoria = crear_conexion(logger_kernel,ip_memoria,puerto_memoria);
    hilo_crear_kernel_memoria = crear_hilo_memoria();//EN ESTA FUNCION DSP CAMBIALE LO QUE LE QUERES PASAR PARA CREAR EL PRIMER HILO/PROCESOS

    //INICIO SERVIDOR KERNEL-CPU
    socket_kernel_cpu_dispatch = iniciar_servidor(logger_kernel, puerto_escucha_dispatch); 
    hilo_escuchar_kernel = escuchar_dispatch_cpu();
    
    socket_kernel_cpu_interrupt= iniciar_servidor(logger_kernel, puerto_escucha_interrupt); 
    hilo_escuchar_kernel_interrupcion = escuchar_interrupcion_cpu();

  
    // El ultimo hilo es con join. Para que este se quede esperando, y no finalize el hilo.
}

void crearEstructuras()
{
    setearAlgoritmosDePlanificacion();

    listaProcesosNew = list_create();
    listaProcesosReady = list_create();
    listaProcesosSwapReady = list_create();

    
    diccionarioIODeProcesosBloqueados = dictionary_create();

    iniciarSemaforosKernel();

}

void setearAlgoritmosDePlanificacion(){
    
    if(strcmp(algoritmo_cola_new,"FIFO")==0)
        algoritmoColaNewEnFIFO=true;
    else if(strcmp(algoritmo_cola_new,"PMCP")==0)
        algoritmoColaNewEnFIFO=false;
    else
        log_error(logger_kernel,"ALGORITMO DE PLANIFICACION DESCONOCIDO");
    

    
    
    if(strcmp(algoritmo_planificacion,"FIFO")==0)
        algoritmoDePlanificacionInt=FIFO;
    else if(strcmp(algoritmo_planificacion,"SJF")==0)
        algoritmoDePlanificacionInt=SJF;
    else if(strcmp(algoritmo_planificacion,"SRT")==0)
        algoritmoDePlanificacionInt=SRT;
    else
        log_error(logger_kernel,"ALGORITMO DE PLANIFICACION DESCONOCIDO");
       

}

void iniciarSemaforosKernel()
{
    semaforoListaNew= malloc(sizeof(sem_t));
    semaforoListaReady = malloc(sizeof(sem_t));
    semaforoListaSwapReady = malloc(sizeof(sem_t));
    
    semaforoDiccionarioIOBlocked = malloc(sizeof(sem_t));
    
    sem_init(semaforoListaNew,1,1);
    sem_init(semaforoListaReady,1,1);


    sem_init(semaforoDiccionarioIOBlocked,1,1); 
    sem_init(semaforoDiccionarioBlocked,1,1); 
    sem_init(semaforoListaSwapReady,1,1);



}

