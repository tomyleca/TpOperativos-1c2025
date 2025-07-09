#include "globalesKernel.h"

sem_t* semaforoMutexPIDDisponible;

//CONFIG Y LOGGER
int socket_kernel_io;
int cliente_kernel;
int socket_kernel_cpu_dispatch;
int socket_kernel_cpu_interrupt;
int cliente_kernel_dispatch;
int cliente_kernel_interrupt;
char* ip_memoria;
char* algoritmo_planificacion;
char* puerto_memoria;
int tiempo_suspension;
char* puerto_escucha_dispatch;
char* puerto_escucha_interrupt;
char* puerto_escucha_IO;
double alfa;
int64_t estimacion_inicial;

char* algoritmo_cola_new;
bool algoritmoColaNewEnFIFO;


t_log_level log_level;
t_log* loggerKernel;

t_config* config_kernel;

pthread_t hilo_escuchar_kernel;
pthread_t hilo_escuchar_kernel_interrupcion;
pthread_t hilo_crear_kernel_memoria;
//PROCESOS
t_listaConSemaforos* listaProcesosNew;
t_listaConSemaforos* listaProcesosReady;
t_listaConSemaforos* listaProcesosSwapReady;
t_listaConSemaforos* listaCPUsAInicializar;
t_listaConSemaforos* listaCPUsLibres;
t_listaConSemaforos* listaCPUsEnUso;
t_listaConSemaforos* listaProcesosPorSerDesalojados;

t_diccionarioConSemaforos* diccionarioProcesosBloqueados;

t_diccionarioConSemaforos* diccionarioProcesosEsperandoDump;

uint32_t pidDisponible = 0;


int algoritmoDePlanificacionInt;



//IO
t_diccionarioConSemaforos* diccionarioDispositivosIO;

//CPU
sem_t* semaforoIntentarPlanificar;
sem_t* semaforoHayCPULibre;
sem_t* semaforoEsperarOKDispatch;
sem_t* semaforoMutexGuardarDatosCPU;
sem_t* semaforoEsperarOKInterrupt;
sem_t* semaforoPCActualizado;
sem_t* semaforoMutexIO;
sem_t* semaforoMutexTerminarEjecucion;
sem_t* semaforoMutexEJecucionTerminada;
sem_t* semaforoEnCheckInterrupt;
sem_t* semaforoInicializarProceso;

//HILOS
pthread_t hiloAtenderDispatch;
pthread_t hiloAtenderInterrupt;
pthread_t hiloAtenderIO;
pthread_t hiloPlanificadorCortoPlazo;

//OTRAS
bool ejecucionYaTerminada;