#include "GlobalesCPU.h"


int socket_cpu_memoria;
int socket_cpu_kernel_dispatch;
int socket_cpu_kernel_interrupt;
char* ip;
char* puerto;
char* valor;
int identificador_cpu;


//Otros
t_log* logger_cpu;
t_config* config_cpu;
char* instruccion_recibida;
bool flag_interrupcion;
int valor_inicial;
op_code motivo_interrupcion;

//HILOS
pthread_t hilo_escuchar_kernel_interrupcion;
pthread_t hilo_escuchar_kernel;
pthread_t hilo_escuchar_memoria;
pthread_t hilo_interpretar_instruccion;

//DICTIONARYS
t_dictionary* registros;
t_dictionary* instrucciones;


t_contexto_cpu* contexto;

//SEMAFOROS
sem_t sem_hay_instruccion;
sem_t sem_pid;
sem_t sem_contexto;


char* ip_memoria;
int puerto_memoria;
char* ip_kernel;
int puerto_kernel_dispatch;
int puerto_kernel_interrupt;
int entradas_tlb;
char* reemplazo_tlb;
int entradas_cache;
char* reemplazo_cache;
int retardo_cache;
t_log_level log_level;