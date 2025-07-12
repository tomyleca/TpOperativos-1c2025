#ifndef IO_H_
#define IO_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils/shared.h"
#include "utils/configs.h"
#include "utils/conexiones.h"
#include "GlobalesIO.h"
#include <semaphore.h>

uint32_t PID;
int64_t tiempo;
sem_t semaforoEmpezarIO;


extern void conectarseAKernel(char* nombre);
extern void leerConfigIO(t_config* config_io);
extern uint32_t recibirProcesoEnIOEIniciarUsleep();
extern void avisarFinDeIO(uint32_t PID,char* nombreIO);

#endif
