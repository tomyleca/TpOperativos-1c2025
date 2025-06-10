#include "GlobalesCPU.h"


int socket_cpu_memoria;
int socket_cpu_kernel_dispatch;
int socket_cpu_kernel_interrupt;
char* ip;
char* puerto;
char* valor;
char* identificador_cpu;


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
pthread_t hilo_crear_kernel_memoria;
pthread_t hilo_escuchar_memoria;
pthread_t hilo_interpretar_instruccion;

//DICTIONARYS
t_dictionary* instrucciones;


t_contexto_cpu* contexto;

//SEMAFOROS
sem_t sem_hay_instruccion;
sem_t semContextoCargado;
sem_t semFetch;
sem_t semOKDispatch;
sem_t sem_interrupcion;
sem_t semMutexPC;




//MMU
int cant_entradas_tabla; 
int tamanio_pagina;              
int cant_niveles;
int timestamp_actual;

EntradaTLB* TLB_proceso;
t_list* lista_tlb;

pthread_mutex_t mutex_motivo_interrupcion;


char* ip_memoria;
char* puerto_memoria;
char* ip_kernel;
char* puerto_kernel_dispatch;
char* puerto_kernel_interrupt;
int entradas_tlb;
char* reemplazo_tlb;
int entradas_cache;
char* reemplazo_cache;
int retardo_cache;
t_log_level log_level;