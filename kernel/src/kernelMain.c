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
    puerto_memoria = config_get_string_value(config_kernel, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT");
    puerto_escucha_IO = config_get_string_value(config_kernel, "PUERTO_ESCUCHA_IO");
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
    INIT_PROC("kernel/pseudocodigo.txt", 4096);

    socket_kernel_cpu_dispatch = iniciar_servidor(logger_kernel, puerto_escucha_dispatch); 
    hilo_escuchar_kernel = escuchar_dispatch_cpu();

    socket_kernel_cpu_interrupt= iniciar_servidor(logger_kernel, puerto_escucha_interrupt); 
    hilo_escuchar_kernel_interrupcion = escuchar_interrupcion_cpu(); 

    pthread_join(hilo_escuchar_kernel,NULL);

    pthread_join(hilo_escuchar_kernel_interrupcion,NULL);
    
    pthread_join(hilo_crear_kernel_memoria,NULL);

  }


void crearEstructuras()
{
    setearAlgoritmosDePlanificacion();

    listaProcesosNew = crearListaConSemaforos();
    listaProcesosReady = crearListaConSemaforos();
    listaProcesosSwapReady = crearListaConSemaforos();


    listaCPUsLibres = crearListaConSemaforos();
    listaCPUsEnUso = crearListaConSemaforos();

    listaDispositivosIO = list_create();

    
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
<<<<<<< HEAD
    
    temporal_stop(proceso->cronometros[estado]);
    proceso->MT[estado]=temporal_gettime(proceso->cronometros[estado]);
}
=======
    semaforoListaNew= malloc(sizeof(sem_t));
    semaforoListaReady = malloc(sizeof(sem_t));
    semaforoListaSwapReady = malloc(sizeof(sem_t));

    semaforoListaDispositivosIO = malloc(sizeof(sem_t));
    
    semaforoDiccionarioIOBlocked = malloc(sizeof(sem_t));
    
    sem_init(semaforoListaNew,1,1);
    sem_init(semaforoListaReady,1,1);


    sem_init(semaforoDiccionarioIOBlocked,1,1); 
    //sem_init(semaforoDiccionarioBlocked,1,1); 
    sem_init(semaforoListaSwapReady,1,1);
    sem_init(semaforoListaDispositivosIO,1,1);



}

>>>>>>> origin/CPU-Eze
