#include "globalesKernel.h"

//CONFIG Y LOGGER
int socket_kernel_io;
int cliente_kernel;
int socket_kernel_cpu_dispatch;
int socket_kernel_cpu_interrupt;
int cliente_kernel_dispatch;
int cliente_kernel_interrupt;
int socket_kernel_memoria;
char* ip_memoria;
char* algoritmo_planificacion;
int puerto_memoria;
int tiempo_suspension;
int puerto_escucha_dispatch;
int puerto_escucha_interrupt;
int puerto_escucha_IO;
int alfa;
char* algoritmo_cola_new;
bool algoritmoColaNewEnFIFO;

t_log_level log_level;
t_log* logger_kernel;

//PROCESOS
t_list* listaProcesosNew;
t_list* listaProcesosReady;
t_list* listaProcesosSwapReady;

t_dictionary* diccionarioProcesosBloqueados;


uint32_t pidDisponible = 0;


int algoritmoDePlanificacionInt;

//IO
t_list* listaDispositivosIO;

sem_t* semaforoListaDispositivosIO;



