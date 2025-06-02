#include "pruebas.h"

void prueba1()
{
    INIT_PROC("1",4);
    INIT_PROC("2",9000000000);
    guardarDatosCPUDispatch("1",1);
    guardarDatosCPUInterrupt("1",3);
    sleep(4);
    syscall_IO(0,"Prueba",7000000);
    sleep(4);
    syscall_IO(1,"IO QUE NO EXISTE",1000);
    sleep(15);
    syscallExit(0);
   
}

void prueba2()
{
    INIT_PROC("1",4);
    INIT_PROC("2",2);
    //guardarDatosCPU("id",1);
    sleep(4);
    syscall_IO(0,"Prueba",4000);
    sleep(4);
    syscall_IO(1,"Prueba2",1000);
    sleep(4);
    syscallExit(0);
   
}

void pruebaConCPU()
{
    INIT_PROC("1",4);
    INIT_PROC("2",2);
}