#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "globalesKernel.h" 
#include "../../utils/src/utils/configs.h"
#include "../../utils/src/utils/shared.h"

void leerConfigKernel(t_config* config_kernel);
void crearEstructuras();
void setearAlgoritmosDePlanificacion();







#endif
