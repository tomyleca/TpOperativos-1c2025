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


typedef struct {
uint32_t PID;
uint32_t PC;
int ME[6];
int MT[6];
char* archivoPseudocodigo;
uint32_t tam;

int64_t estimadoRafagaAnterior; 
int64_t duracionRafagaAnterior;
int64_t estimadoRafagaActual;

} PCB;

typedef enum{
    NEW,
    READY,
    BLOCKED,
    SUSPENDIDO_BLOCKED,
    SUSPENDIDO,
    EXECUTE,
    EXIT
} ESTADO;

typedef enum{
    FIFO,
    SJF,
    SRT
} PLANIFICADOR;


extern int socket_kernel_memoria;

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

extern void INIT_PROC(char* archivoPseudocodigo,unsigned int tam);
extern void inicializarProceso();

//Ordenar listas
extern bool menorTam(PCB* PCB1,PCB* PCB2);
extern bool menorEstimadoRafagaActual(PCB* PCB1,PCB* PCB2);

//Planificador
extern void estimarRafagaActual(PCB* proceso);

//Cambiar de estado
extern void pasarAReady(PCB* proceso);
extern void pasarABLoqueadoEIniciarContador(PCB* proceso);
extern void contadorParaSwap(PCB* proceso);
extern bool IOTerminado(char* PIDComoChar);
extern void pasarASwapBlocked(PCB* proceso, char* PIDComoChar);

//OTRAS
extern char* pasarUnsignedAChar(uint32_t unsigned_);
 






extern t_dictionary* diccionarioProcesosSwapBloqueados;
/**
*@brief Un diccionario que asocia un pid de un proceso bloqueado al estado de la IO que solicito, si la IO no se completo el valor esta en 0, en cambio si se completo esta en 1;
*/
extern t_dictionary* diccionarioIODeProcesosBloqueados;

extern uint32_t pidDisponible;

extern t_list* listaProcesosNew;
extern t_list* listaProcesosReady;
extern t_list* listaProcesosBlocked;


extern int algoritmoDePlanificacionInt;




#endif