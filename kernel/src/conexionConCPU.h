#ifndef CONEXION_CON_CPU_H
#define CONEXION_CON_CPU_H

#include "kernel.h"

/**
 * @brief Guarda los datos de una nueva CPU.
*/
extern  NucleoCPU* guardarDatosCPUDispatch(char* identificador,int fdConexion);


extern void* esperarClientesDispatch(void* arg);

extern void* esperarClientesInterrupt(void* arg);

extern void atender_dispatch_cpu(void* conexion);

extern void esperarDatosInterrupt(void* conexion);

extern NucleoCPU* guardarDatosCPUDispatch(char* identificador,int fdConexion);

extern NucleoCPU* guardarDatosCPUInterrupt(char* identificador,int fdConexion);

extern NucleoCPU* chequearSiCPUYaPuedeInicializarse(char* identificador);

extern void mandarContextoACPU(uint32_t PID,uint32_t PC,int fdConexion);

extern void mandarInterrupcion(NucleoCPU* nucleoCPU);

extern void actualizarPC(uint32_t pid, uint32_t PCActualizado);






#endif