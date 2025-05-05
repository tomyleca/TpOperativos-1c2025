#include "globalesKernel.h"

sem_t* semaforoPIDDisponible;

//CONFIG Y LOGGER
int socket_kernel_io;
int socket_kernel_cpu_dispatch;
int socket_kernel_cpu_interrupt;
int cliente_kernel_dispatch;
int cliente_kernel_interrupt;
int socket_kernel_memoria;


char* ip_memoria;
char* algoritmo_planificacion;
char* puerto_memoria;
int tiempo_suspension;
char* puerto_escucha_dispatch;
char* puerto_escucha_interrupt;
char* puerto_escucha_IO;
int alfa;
char* algoritmo_cola_new;
bool algoritmoColaNewEnFIFO;

t_log_level log_level;
t_log* loggerKernel;
t_config* config_kernel;


pthread_t hilo_escuchar_dispatch;
pthread_t hilo_escuchar_interrupcion;
pthread_t hilo_conectar_kernel_memoria;

//PROCESOS
t_listaConSemaforos* listaProcesosNew;
t_listaConSemaforos* listaProcesosReady;
t_listaConSemaforos* listaProcesosSwapReady;
t_listaConSemaforos* listaCPUsLibres;
t_listaConSemaforos* listaCPUsEnUso;

t_diccionarioConSemaforos* diccionarioProcesosBloqueados;


uint32_t pidDisponible = 0;


int algoritmoDePlanificacionInt;

//IO
t_list* listaDispositivosIO;

sem_t* semaforoListaDispositivosIO;




//IO
t_diccionarioConSemaforos* diccionarioDispositivosIO;

//CPU
sem_t* semaforoIntentarPlanificar;



