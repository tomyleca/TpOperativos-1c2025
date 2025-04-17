#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "globalesKernel.h" 
#include "utils/configs.h"
#include "utils/shared.h"
#include "utils/conexiones.h"
#include "conexiones_kernel.h"

void leerConfigKernel(t_config* config_kernel);
void crearEstructuras();
void setearAlgoritmosDePlanificacion();


void inicializar_hilos(t_config* config_kernel);






#endif
