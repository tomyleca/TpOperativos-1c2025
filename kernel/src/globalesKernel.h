#ifndef GLOBALES_H_
#define GLOBALES_H_

#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <commons/temporal.h>
#include <pthread.h>
#include <commons/collections/dictionary.h>
#include "../../utils/src/utils/monitoresListas.h"
#include "../../utils/src/utils/monitoresDiccionarios.h"
#include "../../utils/src/utils/conexionKernelIO.h"



typedef struct {
uint32_t PID;
uint32_t PC;

char* archivoPseudocodigo;
uint32_t tam;

int64_t estimadoRafagaAnterior; 
int64_t duracionRafagaAnterior;
int64_t estimadoRafagaActual;

t_temporal* cronometros[7];
t_temporal* cronometroEjecucionActual;


int ME[7];
int MT[7];


} PCB;

typedef enum{
    NEW,
    READY,
    BLOCKED,
    SWAP_BLOCKED,
    SWAP_READY,
    EXECUTE,
    EXIT
} ESTADO;

//HILOS
extern pthread_t hilo_escuchar_kernel;
extern pthread_t hilo_escuchar_kernel_interrupcion;
extern pthread_t hilo_conectar_kernel_memoria;
extern pthread_t hilo_crear_kernel_memoria;
typedef enum{
    FIFO,
    SJF,
    SRT
} PLANIFICADOR;


typedef struct
{
    char* nombre;
    bool ocupado;
    int fdConexion;
} DispositivoIO;

typedef struct{
    PCB* proceso;
    bool IOFinalizada;
    sem_t* semaforoIOFinalizada;
    bool estaENSwap;
} procesoEnEsperaIO;


typedef struct{
    char* identificador;
    bool ejecutando;
    PCB* procesoEnEjecucion;
    int fdConexion;
    
    
}
nucleoCPU;



extern int socket_kernel_memoria;
extern int socket_kernel_io;
extern int cliente_kernel;
extern int socket_kernel_cpu_dispatch;
extern int socket_kernel_cpu_interrupt;
extern int cliente_kernel_dispatch;
extern int cliente_kernel_interrupt;

//CONFIG Y LOGGER
extern char* ip_memoria;
extern char* algoritmo_planificacion;
extern int puerto_memoria;
extern int tiempo_suspension;
extern int puerto_escucha_dispatch;
extern int puerto_escucha_interrupt;
extern int puerto_escucha_IO;
extern int alfa;
extern char*  algoritmo_cola_new;
extern bool algoritmoColaNewEnFIFO;

extern t_log_level log_level;
extern t_log* logger_kernel;


//PROCESOS
extern uint32_t pidDisponible;

extern void INIT_PROC(char* archivoPseudocodigo,unsigned int tam);
extern void inicializarProceso();

//Ordenar listas
extern bool menorTam(PCB* PCB1,PCB* PCB2);
extern bool menorEstimadoRafagaActual(PCB* PCB1,PCB* PCB2);

//Planificador
extern void estimarRafagaActual(PCB* proceso);
extern void* planificadorCortoPlazo(void* arg);
extern void ejecutar(PCB* proceso);

/**
 * @brief Chequea si en alguno de los CPUs en ejecución hay un proceso con una rafaga estimada restante menor a la rafaga estimada más baja de los procesos en ready. De ser así, libera el CPU y retorna true, de otra forma retorna false.
*/
extern bool chequearSiHayDesalojo(int64_t estimadoRafagaProcesoEnEspera);
extern bool menorEstimadoRafagaRestante(nucleoCPU* CPU1,nucleoCPU* CPU2);

extern PCB* terminarEjecucionNucleoCPU(nucleoCPU* nucleoCPU);
extern void guardarDatosDeEjecucion(PCB* procesoDespuesDeEjecucion);

extern t_listaConSemaforos* listaProcesosNew;
extern t_listaConSemaforos* listaProcesosReady;
extern t_listaConSemaforos* listaProcesosSwapReady;
extern t_listaConSemaforos* listaCPUsLibres;
extern t_listaConSemaforos* listaCPUsEnUso;

extern t_diccionarioConSemaforos* diccionarioProcesosBloqueados;

extern int algoritmoDePlanificacionInt;


//CONEXIONES
extern void iniciarConexiones();
extern void cerrarConexiones();

//IO


extern t_listaConSemaforos* listaDispositivosIO;

//CPU
extern sem_t* semaforoIntentarPlanificar;


//OTRAS
extern char* pasarUnsignedAChar(uint32_t unsigned_);

 













#endif