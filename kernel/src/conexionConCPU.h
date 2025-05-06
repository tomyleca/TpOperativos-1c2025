#ifndef CONEXION_CON_CPU_H
#define CONEXION_CON_CPU_H

#include "kernel.h"

/**
 * @brief Guarda los datos de una nueva CPU.
*/
extern  void guardarDatosCPU(char* identificador,int fdConexion);

/**
 * @brief Loggea las métricas de estado antes de finalizar el proceso
*/
extern void loggearMetricas(PCB* proceso);
 
#endif