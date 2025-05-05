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


extern void conectarseAKernel(char* nombre);
<<<<<<< HEAD
extern void leerConfigIO(t_config* config_io);
extern uint32_t recibirProcesoEnIOEIniciarUsleep();
extern void avisarFinDeIO(uint32_t PID,char* nombreIO);
=======
void leerConfigIO(t_config* config_io);
>>>>>>> origin/CPU-Eze

#endif
