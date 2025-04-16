#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "globalesKernel.h" 
#include "../../utils/src/utils/configs.h"
#include"../../utils/src/utils/shared.h"

typedef struct{
    uint32_t pid;
    char* archivoDePseudocodigo;
    t_list* listaDeHilosDelProceso;
    t_list* listaDeMutexDelProceso;
    ESTADO estadoDelProceso;
    uint32_t tamanio;
    uint32_t siguienteTid;
    uint32_t siguienteMid;
    uint32_t prioridadTid0;
}t_pcb;

typedef struct t_tcb{
    uint32_t tid;
    char* archivoDePseudocodigo;
    t_pcb* procesoPadre;
    ESTADO estadoDelHilo;
    uint32_t prioridad;
    t_tcb* hiloBloqueadoPorJoin; //Hilo al que estoy bloqueando
    t_list* mutexEnEspera; //Mutex a los cuales estoy esperando
    bool bloqueadoPorJoin; //Para saber si estoy esperando algun hilo
    bool bloqueadoPorIO;
    uint32_t finDeq;
    bool clockContando;
}t_tcb;



#endif