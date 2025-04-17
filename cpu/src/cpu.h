#ifndef CPU_H_
#define CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils/configs.h"
#include "utils/shared.h"
#include "utils/conexiones.h"
#include "GlobalesCPU.h"
#include "conexion-cpu-memoria.h"

void leerConfigCpu(t_config* config_cpu);
void inicializar_hilos(t_config* config_cpu);



#endif
