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
#include "conexionConCPU.h"
#include "utils/configs.h"
#include "utils/shared.h"
#include "utils/conexiones.h"
#include "pruebas.h"
#include "syscalls.h"
#include "pruebas.h"
#include <signal.h>

void leerConfigKernel(t_config* config_kernel);
void crearEstructuras();
void setearAlgoritmosDePlanificacion();

//IO
/**
 * @brief Para cada nuevo dispositivo IO que se ejecuta se crea una nueva conexion(mediante esperar_cliente),luego se le crea un hilo donde se atienden todas las peticiones de esa conexión.
*/
extern void* esperarClientesIO(void* arg);
extern void* atenderInstanciaIO(void* conexion);

extern void avisarInicioIO(ProcesoEnEsperaIO* procesoEnEsperaIO,char* nombreIO,int64_t tiempo);
extern DispositivoIO* buscarIOSegunNombre(char* nombreIO);

extern void cargarCronometro(PCB* proceso, ESTADO estado);


void inicializar_hilos_kernel(t_config* config_kernel);
void liberarRecursos(int signal);





#endif
