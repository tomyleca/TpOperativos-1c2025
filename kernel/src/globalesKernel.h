#ifndef GLOBALES_H_
#define GLOBALES_H_

#include<stdio.h>
#include<stdlib.h>
#include <stdint.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "../../utils/src/utils/monitoresListas.h"


typedef struct {
uint32_t PID;
uint32_t PC;
int ME[6];
int MT[6];
char* archivoPseudocodigo;
uint32_t tam;
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
void inicializarProceso();
bool menorTam(PCB* PCB1,PCB* PCB2);

extern uint32_t pidDisponible;

extern t_list* listaProcesosNew;
extern t_list* listaProcesosReady;


extern int algoritmoDePlanificacionInt;




#endif