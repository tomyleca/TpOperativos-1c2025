#include "globalesKernel.h"

void nucleoCPUDestroy(void* ptr)
{
    if(ptr==NULL)
        return;
    
    NucleoCPU* nucleoCPU = (NucleoCPU*) ptr;
    free(nucleoCPU->identificador);
    free(nucleoCPU);
}

void dispositivoIODestroy(void* ptr)
{
    if(ptr==NULL)
        return;
    
    DispositivoIO* dispositivoIO = (DispositivoIO*) ptr;
    free(dispositivoIO->nombre);
    list_destroy(dispositivoIO->colaEsperandoIO);
    list_destroy_and_destroy_elements(dispositivoIO->listaInstancias,instanciaIODestroy);
    free(dispositivoIO);
}

void instanciaIODestroy(void* ptr)
{
    if(ptr==NULL)
        return;

    InstanciaIO* instanciaIO = (InstanciaIO*) ptr;
    free(instanciaIO->semaforoMutex);
    free(instanciaIO);
}

void procesoEnEsperaIODestroy(void* ptr)
{
    if(ptr==NULL)
        return;

    ProcesoEnEsperaIO* procesoEnEsperaIO = (ProcesoEnEsperaIO*) ptr;
    free(procesoEnEsperaIO->proceso);
    free(procesoEnEsperaIO->semaforoIOFinalizada);
    free(procesoEnEsperaIO->semaforoMutex);
    free(procesoEnEsperaIO);
}

void procesoEnEsperaDumpDestroy(void* ptr)
{
    if(ptr==NULL)
        return;

    ProcesoEnEsperaDump* procesoEnEsperaDump = (ProcesoEnEsperaDump*) ptr;
    free(procesoEnEsperaDump->semaforoDumpFinalizado);
    free(procesoEnEsperaDump->semaforoMutex);
    free(procesoEnEsperaDump);
}