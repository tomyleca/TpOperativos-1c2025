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
t_log_level log_level;
t_log* logger_kernel;
int puerto_escucha_dispatch;
int puerto_escucha_interrupt;
int puerto_escucha_IO;


pthread_t hilo_escuchar_kernel;
pthread_t hilo_escuchar_kernel_interrupcion;
pthread_t hilo_crear_kernel_memoria;
//PROCESOS

