#include "instruccion.h"

void syscall_IO(char** parte){
    t_paquete* paquete = crear_super_paquete(IO);
    cargar_string_al_super_paquete(paquete,parte[1]);
    int64_t tiempo = (int64_t) strtoul(parte[2], NULL, 10);
    cargar_int64_t_al_super_paquete(paquete,tiempo);
    enviar_paquete(paquete,socket_cpu_kernel_dispatch);
    esperarOK2(socket_cpu_kernel_dispatch);
}