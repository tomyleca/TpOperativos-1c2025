#include <kernel.h>

int main(int argc, char* argv[]) {

    //INICIO Y LEO CONFIG
    config_kernel = iniciar_config("kernel.config");
    leerConfigKernel(config_kernel);
    
    //INICIO LOGGER
    logger_kernel = iniciar_logger("kernelLogger.log","kernelLogger",log_level);

    //ME FIJO CUALES SON LOS ALGORITMOS DE PLANIFICACION/ CREO LAS LISTAS PARA MANEJAR PROCESOS/ INICIALIZO LOS SEMAFOROS
    crearEstructuras();
    
    prueba1();
    
   

  

    //inicializar_hilos_kernel(config_kernel);

   

    //INIT_PROC("1",4);
    //INIT_PROC("2",2);
    //guardarDatosCPU("id",1);

    
    return 0;
}

void leerConfigKernel(t_config* config_kernel) {
    
    ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config_kernel, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT");
    puerto_escucha_IO = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_IO");
    algoritmo_planificacion = config_get_string_value(config_kernel, "ALGORITMO_CORTO_PLAZO");
    algoritmo_cola_new = config_get_string_value(config_kernel, "ALGORITMO_INGRESO_A_READY");
    alfa = config_get_int_value(config_kernel, "ALFA");
    tiempo_suspension = config_get_int_value(config_kernel, "TIEMPO_SUSPENSION");
    log_level = log_level_from_string(config_get_string_value(config_kernel, "LOG_LEVEL"));
    
}

void inicializar_hilos_kernel(t_config* config_kernel)
{
    /*pthread_t* hiloAtenderIO = malloc(sizeof(pthread_t));
    pthread_t* hiloPlanificadorCortoPlazo = malloc(sizeof(pthread_t));
    pthread_create(hiloAtenderIO,NULL,esperarClientesIO,NULL);
    pthread_create(hiloPlanificadorCortoPlazo,NULL,planificadorCortoPlazo,NULL);
    pthread_join(*hiloAtenderIO,NULL);
    pthread_join(*hiloPlanificadorCortoPlazo,NULL);*/

    socket_kernel_memoria = crear_conexion(logger_kernel, ip_memoria, puerto_memoria);
    hilo_conectar_kernel_memoria = crear_hilo_memoria();

    INIT_PROC("../memoria/pseudocodigo.txt", 4096);

    socket_kernel_cpu_dispatch = iniciar_servidor(logger_kernel, puerto_escucha_dispatch); 
    log_info(logger_kernel, "Servidor DISPATCH iniciado");
    socket_kernel_cpu_interrupt = iniciar_servidor(logger_kernel, puerto_escucha_interrupt); 
    log_info(logger_kernel, "Servidor INTERRUPT iniciado");

    hilo_escuchar_dispatch = escuchar_dispatch_cpu();
	hilo_escuchar_interrupcion = escuchar_interrupcion_cpu();

    pthread_join(hilo_escuchar_dispatch,NULL);
    pthread_join(hilo_escuchar_interrupcion,NULL);
    pthread_join(hilo_conectar_kernel_memoria,NULL);

}


void crearEstructuras()
{
    setearAlgoritmosDePlanificacion();

    listaProcesosNew = crearListaConSemaforos();
    listaProcesosReady = crearListaConSemaforos();
    listaProcesosSwapReady = crearListaConSemaforos();


    listaCPUsLibres = crearListaConSemaforos();
    listaCPUsEnUso = crearListaConSemaforos();

    
    diccionarioDispositivosIO = crearDiccionarioConSemaforos();
    diccionarioProcesosBloqueados = crearDiccionarioConSemaforos();

    semaforoIntentarPlanificar = malloc(sizeof(sem_t));
    sem_init(semaforoIntentarPlanificar,1,0);
    semaforoPIDDisponible = malloc(sizeof(sem_t));
    sem_init(semaforoPIDDisponible,1,1);



    

    
}

void setearAlgoritmosDePlanificacion(){
    
    if(strcmp(algoritmo_cola_new,"FIFO")==0)
        algoritmoColaNewEnFIFO=true;
    else if(strcmp(algoritmo_cola_new,"PMCP")==0)
        algoritmoColaNewEnFIFO=false;
    else
        log_error(loggerKernel,"ALGORITMO DE PLANIFICACION DESCONOCIDO");
    

    
    
    if(strcmp(algoritmo_planificacion,"FIFO")==0)
        algoritmoDePlanificacionInt=FIFO;
    else if(strcmp(algoritmo_planificacion,"SJF")==0)
        algoritmoDePlanificacionInt=SJF;
    else if(strcmp(algoritmo_planificacion,"SRT")==0)
        algoritmoDePlanificacionInt=SRT;
    else
        log_error(loggerKernel,"ALGORITMO DE PLANIFICACION DESCONOCIDO");
       

}


void cargarCronometro(PCB* proceso,ESTADO estado)
{
    
    temporal_stop(proceso->cronometros[estado]);
    proceso->MT[estado]=temporal_gettime(proceso->cronometros[estado]);
}