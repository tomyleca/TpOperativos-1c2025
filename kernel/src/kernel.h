#ifndef KERNEL_H_
#define KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "globalesKernel.h" 
#include "cambiarDeEstado.h"
#include "utils/configs.h"
#include "utils/shared.h"
#include "utils/conexiones.h"

void leerConfigKernel(t_config* config_kernel);
void crearEstructuras();
void setearAlgoritmosDePlanificacion();

//IO
/**
 * @brief Para cada nuevo dispositivo IO que se ejecuta se crea una nueva conexion(mediante esperar_cliente),luego se le crea un hilo donde se atienden todas las peticiones de esa conexión.
*/
extern void* esperarClientesIO(void* arg);
extern void* atenderIO(int* conexion);


extern void avisarInicioIO(uint32_t PID,char* nombreIO,uint32_t tiempo);
extern DispositivoIO* buscarIOSegunNombre(char* nombreIO);







#endif
