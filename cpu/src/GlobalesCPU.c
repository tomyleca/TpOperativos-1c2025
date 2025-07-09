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
t_contexto_cpu* contextoAnterior;


//SEMAFOROS
sem_t sem_hay_instruccion;
sem_t semContextoCargado;
sem_t semFetch;
sem_t semOKDispatch;
sem_t sem_interrupcion;
sem_t semMutexContexto;
sem_t semLlegoPeticionMMU;
sem_t semOkEscritura;
sem_t semLlegoPeticionTabla;

sem_t sem_pagina_recibida;
sem_t sem_pagina_escrita;
sem_t sem_valor_leido;
sem_t mutex_cache;
sem_t semLlegoPeticionTabla;
sem_t semFinCicloInstruccion;


//MMU
int cant_entradas_tabla; 
int tamanio_pagina;              
int cant_niveles;
int timestamp_actual;
int nro_marco;

int pid_lectura;
int dir_fisica_lectura;
char* valor_str_temp;

EntradaTLB* TLB_proceso;

// *********CACHE**********
EntradaCache* cache_paginas;
int puntero_clock; 
int pid_pagina;
int nro_pagina_recibida;
int nro_marco_recibido;
int contenido_recibido;


t_list* lista_tlb;

sem_t mutex_motivo_interrupcion;
sem_t mutex_lista_tlb;
sem_t llegaron_tam;


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

//CACHE

char valor_leido_memoria[256]; //TODO ver si esto no puede traer problemas
char* buffer_pagina_recibida;
char* buffer_intermedio_pagina_recibida;
