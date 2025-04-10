#include "globalesKernel.h"

//CONFIG Y LOGGER
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

uint32_t pidDisponible = 0;


int algoritmoDePlanificacionInt;


