#include <kernel.h>

int main(int argc, char* argv[]) {

    signal(SIGINT,liberarRecursos);

    //INICIO Y LEO CONFIG
    config_kernel = iniciar_config("kernel.config");
    leerConfigKernel(config_kernel);
    
    //INICIO LOGGER
    loggerKernel = iniciar_logger("kernelLogger.log","kernelLogger",log_level);

    //INICIO SEVIDORES PARA COMUNICACION IO-KERNEL Y CPU-KERNEL
    iniciarServidores();

    //ME FIJO CUALES SON LOS ALGORITMOS DE PLANIFICACION/ CREO LAS LISTAS PARA MANEJAR PROCESOS/ INICIALIZO LOS SEMAFOROS
    crearEstructuras();

    

    pthread_create(&hiloAtenderDispatch,NULL,esperarClientesDispatch,NULL);
    pthread_create(&hiloAtenderInterrupt,NULL,esperarClientesInterrupt,NULL);
    pthread_create(&hiloAtenderIO,NULL,esperarClientesIO,NULL);
    



    if(argc == 3)
    {
        INIT_PROC(argv[1], (uint32_t) strtoul(argv[2], NULL, 10)); // Paso el char a uint32_t
    }
    else
    {
        log_error(loggerKernel,"## (<%u>) ERROR. No se pasaron los parámetros suficientes para iniciar el primer proceso",0);
        exit(1);
    }

    pthread_create(&hiloPlanificadorCortoPlazo,NULL,planificadorCortoPlazo,NULL);

    
   

  


    

    


    pthread_join(hiloAtenderIO,NULL);
    pthread_join(hiloPlanificadorCortoPlazo,NULL);
    pthread_join(hiloAtenderDispatch,NULL);
    pthread_join(hiloAtenderInterrupt,NULL);
    return 0;
}

void leerConfigKernel(t_config* config_kernel) {
    
    ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
    puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT");
    puerto_escucha_IO = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_IO");
    algoritmo_planificacion = config_get_string_value(config_kernel, "ALGORITMO_CORTO_PLAZO");
    algoritmo_cola_new = config_get_string_value(config_kernel, "ALGORITMO_INGRESO_A_READY");
    alfa = config_get_double_value(config_kernel, "ALFA");
    tiempo_suspension = config_get_int_value(config_kernel, "TIEMPO_SUSPENSION");
    log_level = log_level_from_string(config_get_string_value(config_kernel, "LOG_LEVEL"));
    estimacion_inicial = config_get_int_value(config_kernel,"ESTIMACION_INICIAL");
    
}



void crearEstructuras()
{
    setearAlgoritmosDePlanificacion();

    listaProcesosNew = crearListaConSemaforos();
    listaProcesosReady = crearListaConSemaforos();
    listaProcesosSwapReady = crearListaConSemaforos();

    listaCPUsAInicializar = crearListaConSemaforos();
    listaCPUsLibres = crearListaConSemaforos();
    listaCPUsEnUso = crearListaConSemaforos();

    listaProcesosPorSerDesalojados = crearListaConSemaforos();

    
    diccionarioDispositivosIO = crearDiccionarioConSemaforos();
    diccionarioProcesosBloqueados = crearDiccionarioConSemaforos();
    diccionarioProcesosEsperandoDump = crearDiccionarioConSemaforos();

    
    semaforoEsperarOKDispatch = malloc(sizeof(sem_t));
    sem_init(semaforoEsperarOKDispatch,1,0);
    semaforoEsperarOKInterrupt = malloc(sizeof(sem_t));
    sem_init(semaforoEsperarOKInterrupt,1,0);
    semaforoIntentarPlanificar = malloc(sizeof(sem_t));
    sem_init(semaforoIntentarPlanificar,1,0);
    semaforoHayCPULibre = malloc(sizeof(sem_t));
    sem_init(semaforoHayCPULibre,1,0);
    semaforoMutexPIDDisponible = malloc(sizeof(sem_t));
    sem_init(semaforoMutexPIDDisponible,1,1);
    semaforoMutexGuardarDatosCPU = malloc(sizeof(sem_t));
    sem_init(semaforoMutexGuardarDatosCPU,1,1);
    semaforoPCActualizado = malloc(sizeof(sem_t));
    sem_init(semaforoPCActualizado,1,0);
    semaforoMutexIO = malloc(sizeof(sem_t));
    sem_init(semaforoMutexIO,1,1);
    semaforoMutexTerminarEjecucion = malloc(sizeof(sem_t));
    sem_init(semaforoMutexTerminarEjecucion,1,1);


  


    

    
}

void setearAlgoritmosDePlanificacion(){
    
    if(strcmp(algoritmo_cola_new,"FIFO")==0)
        algoritmoColaNewEnFIFO=true;
    else if(strcmp(algoritmo_cola_new,"PMCP")==0)
        algoritmoColaNewEnFIFO=false;
    else
        log_error(loggerKernel,"## ALGORITMO DE PLANIFICACION DESCONOCIDO");
    

    
    
    if(strcmp(algoritmo_planificacion,"FIFO")==0)
        algoritmoDePlanificacionInt=FIFO;
    else if(strcmp(algoritmo_planificacion,"SJF")==0)
        algoritmoDePlanificacionInt=SJF;
    else if(strcmp(algoritmo_planificacion,"SRT")==0)
        algoritmoDePlanificacionInt=SRT;
    else
        log_error(loggerKernel,"## ALGORITMO DE PLANIFICACION DESCONOCIDO");
       

}


void cargarCronometro(PCB* proceso,ESTADO estado)
{
    
    temporal_stop(proceso->cronometros[estado]);
    proceso->MT[estado]=temporal_gettime(proceso->cronometros[estado]);
}

void liberarRecursos(int signal)
{
    if(signal != SIGINT)
        return;
    
   
    //TODO probarlo

    close(socket_kernel_cpu_dispatch);
    close(socket_kernel_cpu_interrupt);
    close(socket_kernel_io);

    free(semaforoEsperarOKDispatch);
    free(semaforoEsperarOKInterrupt);
    free(semaforoMutexGuardarDatosCPU);
    free(semaforoHayCPULibre);
    free(semaforoIntentarPlanificar);
    free(semaforoMutexPIDDisponible);

    pthread_cancel(hiloAtenderDispatch);
    pthread_cancel(hiloAtenderInterrupt);
    pthread_cancel(hiloAtenderIO);
    pthread_cancel(hiloPlanificadorCortoPlazo);


    //TODO hacer los destroys
    /*
    borrarListaConSemaforos(listaCPUsAInicializar);

    borrarListaConSemaforos(listaProcesosNew);
    borrarListaConSemaforos(listaProcesosReady);
    borrarListaConSemaforos(listaProcesosSwapReady);
    */

    destruirDiccionario(diccionarioDispositivosIO,dispositivoIODestroy);
    destruirDiccionario(diccionarioProcesosBloqueados,procesoEnEsperaIODestroy);
    destruirDiccionario(diccionarioProcesosEsperandoDump,procesoEnEsperaDumpDestroy);
    //borrarListaConSemaforos(listaCPUsEnUso,nucleoCPUDestroy); //TODO implementar otra funcion con element destroyer
    //borrarListaConSemaforos(listaCPUsLibres,nucleoCPUDestroy);

    exit(1);
}