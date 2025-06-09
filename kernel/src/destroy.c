#include "globalesKernel.h"

void nucleoCPUDestroy(void* ptr)
{
    NucleoCPU* nucleoCPU = (NucleoCPU*) ptr;
    free(nucleoCPU->identificador);
    free(nucleoCPU);
}

void dispositivoIODestroy(void* ptr)
{
    DispositivoIO* dispositivoIO = (DispositivoIO*) ptr;
    free(dispositivoIO->nombre);
    list_destroy(dispositivoIO->colaEsperandoIO);
    list_destroy_and_destroy_elements(dispositivoIO->listaInstancias,instanciaIODestroy);
    free(dispositivoIO);
}

void instanciaIODestroy(void* ptr)
{
    InstanciaIO* instanciaIO = (InstanciaIO*) ptr;
    free(instanciaIO->semaforoMutex);
    free(instanciaIO);
}

void procesoEnEsperaIODestroy(void* ptr)
{
    ProcesoEnEsperaIO* procesoEnEsperaIO = (ProcesoEnEsperaIO*) ptr;
    free(procesoEnEsperaIO->proceso);
    free(procesoEnEsperaIO->semaforoIOFinalizada);
    free(procesoEnEsperaIO->semaforoMutex);
    free(procesoEnEsperaIO);
}