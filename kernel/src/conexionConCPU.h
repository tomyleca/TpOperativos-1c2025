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

extern void* esperarClientesDispatch(void* arg);

extern void* esperarClientesInterrupt(void* arg);

extern void atender_dispatch_cpu(void* conexion);

extern void atender_interrupcion_cpu(void* conexion);

extern void mandarContextoACPU(uint32_t PID,uint32_t PC,int fdConexion);




#endif