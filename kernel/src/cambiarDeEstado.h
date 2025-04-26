#ifndef CAMBIAR_DE_ESTADO_
#define CAMBIAR_DE_ESTADO_
#include "globalesKernel.h"


extern void pasarAReady(PCB* proceso);
extern void pasarABLoqueadoEIniciarContador(PCB* proceso,uint32_t tiempo,char* nombreIO);
extern void* contadorParaSwap(PCB* proceso);
extern bool IOTerminado(char* PIDComoChar);
extern void pasarASwapBlocked(procesoEnEsperaIO* procesoEsperandoIO);
extern void pasarASwapReady(PCB* proceso);
extern void pasarAExecute(PCB* proceso);
extern void pasarAExit(PCB* proceso);

#endif