#include "pruebas.h"

void prueba1()
{
    INIT_PROC("1",4);
    INIT_PROC("2",2);
    guardarDatosCPU("id",1);
    sleep(10);
    PCB* proceso = terminarEjecucionNucleoCPU(leerDeLista(listaCPUsEnUso,0));
    pasarABLoqueado(proceso,4700000,"Prueba");
}