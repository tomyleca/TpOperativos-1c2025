#ifndef CAMBIAR_DE_ESTADO_
#define CAMBIAR_DE_ESTADO_
#include "globalesKernel.h"


extern void pasarAReady(PCB* proceso,bool desalojado);
extern void pasarABLoqueadoPorIO(PCB* proceso,int64_t tiempo,char* nombreIO);
extern void* manejarProcesoBloqueadoPorIO(ProcesoEnEsperaIO* ProcesoEnEsperaIO);
extern bool dispositivoOcupado (DispositivoIO* dispositivoIO);
extern bool instanciaLibre(InstanciaIO* instanciaIO);
extern void manejarFinDeIO(uint32_t PID,char* nombreDispositivoIO,int fdConexion);
extern void empezarIODelProximoEnEspera(DispositivoIO* dispositivoIO);
extern void contadorParaSwap (ProcesoEnEsperaIO* ProcesoEnEsperaIO);
extern void pasarASwapBlocked(ProcesoEnEsperaIO* procesoEsperandoIO);
extern void pasarASwapReady(PCB* proceso);
extern void pasarAExecute(PCB* proceso);
extern bool chequearSiHayDesalojo(int64_t estimadoRafagaProcesoEnEspera);
extern void pasarAExit(PCB* proceso,char* estadoActual);

#endif