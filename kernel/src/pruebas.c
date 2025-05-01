#include "pruebas.h"

void prueba1()
{
    INIT_PROC("1",4);
    INIT_PROC("2",2);
    guardarDatosCPU("id",1);
    sleep(4);
    syscall_IO(0,"Prueba",1);
   
}