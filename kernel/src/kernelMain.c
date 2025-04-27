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

    pthread_t* hiloAtenderIO = malloc(sizeof(pthread_t));
    pthread_t* hiloPlanificadorCortoPlazo = malloc(sizeof(pthread_t));
    pthread_create(hiloAtenderIO,NULL,esperarClientesIO,NULL);
    pthread_create(hiloPlanificadorCortoPlazo,NULL,planificadorCortoPlazo,NULL);

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

    

    
   

    INIT_PROC("1",4);
    INIT_PROC("2",2);
    guardarDatosCPU("id",1);

    

    


    pthread_join(*hiloAtenderIO,NULL);
    pthread_join(*hiloPlanificadorCortoPlazo,NULL);
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


void crearEstructuras()
{
    setearAlgoritmosDePlanificacion();

    listaProcesosNew = crearListaConSemaforos();
    listaProcesosReady = crearListaConSemaforos();
    listaProcesosSwapReady = crearListaConSemaforos();

    listaDispositivosIO = crearListaConSemaforos();

    listaCPUsLibres = crearListaConSemaforos();
    listaCPUsEnUso = crearListaConSemaforos();

    
    diccionarioProcesosBloqueados = crearDiccionarioConSemaforos();

    semaforoIntentarPlanificar = malloc(sizeof(sem_t));
    sem_init(semaforoIntentarPlanificar,1,0);

    

    
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


void cargarCronometro(PCB* proceso,ESTADO estado)
{
    
    temporal_stop(proceso->cronometros[estado]);
    proceso->MT[estado]=temporal_gettime(proceso->cronometros[estado]);
}