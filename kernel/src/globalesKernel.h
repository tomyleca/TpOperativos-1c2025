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
#include "../../utils/src/utils/conexiones.h"
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
int64_t estimadoSiguienteRafaga;

t_temporal* cronometros[7];
t_temporal* cronometroEjecucionActual;


int ME[7];
int MT[7];


} PCB;

extern sem_t* semaforoPIDDisponible;

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
    sem_t* semaforoDispositivoOcupado;
    t_listaConSemaforos* colaEsperandoIO;
    int fdConexion;
} DispositivoIO;

typedef struct{
    PCB* proceso;
    sem_t* semaforoIOFinalizada;
    bool estaENSwap;
    int64_t tiempo;
} procesoEnEsperaIO;


typedef struct{
    char* identificador;
    bool ejecutando;
    PCB* procesoEnEjecucion;
    int fdConexionDispatch;
    int fdConexionInterrupt;
    
    
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
extern char* puerto_memoria;
extern int tiempo_suspension;
extern char* puerto_escucha_dispatch;
extern char* puerto_escucha_interrupt;
extern char* puerto_escucha_IO;
extern int alfa;
extern char*  algoritmo_cola_new;
extern bool algoritmoColaNewEnFIFO;

extern t_log_level log_level;
extern t_log* loggerKernel;


extern t_config* config_kernel;


//PROCESOS
extern uint32_t pidDisponible;

/**
 * @brief Se fija cual es el proximo proceso para pasar intentar pasar a Ready. Le consulta a memoria y ,si esta da el OK,lo pasa. De otra forma no hace nada.
*/
extern void inicializarProceso();

PCB* buscarPCBEjecutando(uint32_t pid);

//Ordenar listas
extern bool menorTam(PCB* PCB1,PCB* PCB2);
extern bool menorEstimadoSiguienteRafaga(PCB* PCB1,PCB* PCB2);

//Planificador
/**
 * @brief Estima el valor de la proxima ráfaga de un proceso dado.
*/
extern void estimarSiguienteRafaga(PCB* proceso);
extern void* planificadorCortoPlazo(void* arg);


/**
 * @brief Chequea si en alguno de los CPUs en ejecución hay un proceso con una rafaga estimada restante menor a la rafaga estimada más baja de los procesos en ready. De ser así, libera el CPU y retorna true, de otra forma retorna false.
*/
extern bool chequearSiHayDesalojo(int64_t estimadoRafagaProcesoEnEspera);
extern bool menorEstimadoRafagaRestante(nucleoCPU* CPU1,nucleoCPU* CPU2);

extern void terminarEjecucion(PCB* proceso);
extern void guardarDatosDeEjecucion(PCB* procesoDespuesDeEjecucion);

extern t_listaConSemaforos* listaProcesosNew;
extern t_listaConSemaforos* listaProcesosReady;
extern t_listaConSemaforos* listaProcesosSwapReady;
extern t_listaConSemaforos* listaCPUsAInicializar;
extern t_listaConSemaforos* listaCPUsLibres;
extern t_listaConSemaforos* listaCPUsEnUso;

extern t_diccionarioConSemaforos* diccionarioProcesosBloqueados;

extern int algoritmoDePlanificacionInt;


//CONEXIONES
extern void iniciarServidores();
extern void cerrarConexiones();
extern bool esperarOK2(int fdConexion);

//IO


extern t_diccionarioConSemaforos* diccionarioDispositivosIO;

//CPU
extern sem_t* semaforoIntentarPlanificar;

extern sem_t* semaforoGuardarDatosCPU;

//MEMORIA
extern void mandarDatosProcesoAMemoria(PCB* proceso);


//OTRAS


 










#endif