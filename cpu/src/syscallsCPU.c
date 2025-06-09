#include "instruccion.h"


void syscall_IO(char** parte){
    t_paquete* paquete = crear_super_paquete(IO);
    cargar_uint32_t_al_super_paquete(paquete, contexto->pid);
    cargar_uint32_t_al_super_paquete(paquete, contexto->registros.PC + 1);
    cargar_string_al_super_paquete(paquete,parte[1]);
    int64_t tiempo = (int64_t) strtoll(parte[2], NULL, 10);
    cargar_int64_t_al_super_paquete(paquete,tiempo);
    enviar_paquete(paquete,socket_cpu_kernel_dispatch);
    //eliminar_paquete(paquete);
    esperarOK(socket_cpu_kernel_dispatch);
}

void syscallINIT_PROC(char** parte)
{
    
    printf("llego hasta aca");
    t_paquete* paquete = crear_super_paquete(INIT_PROCCESS);
    cargar_uint32_t_al_super_paquete(paquete, contexto->pid);
    cargar_string_al_super_paquete(paquete, parte[1]);
    cargar_uint32_t_al_super_paquete(paquete,(uint32_t) strtoul(parte[2], NULL, 10));
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    
    //eliminar_paquete(paquete);
    esperarOK(socket_cpu_kernel_dispatch);
}

void syscallDUMP_MEMORY(char** parte)
{
    t_paquete* paquete = crear_super_paquete(DUMP_MEMORY);
    cargar_uint32_t_al_super_paquete(paquete, contexto->pid);
    cargar_uint32_t_al_super_paquete(paquete, contexto->registros.PC + 1);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    esperarOK(socket_cpu_kernel_dispatch);
}

void syscallEXIT(char** parte)
{
    t_paquete* paquete = crear_super_paquete(SYSCALL_EXIT);
    cargar_uint32_t_al_super_paquete(paquete, contexto->pid);
    cargar_uint32_t_al_super_paquete(paquete, contexto->registros.PC + 1);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    esperarOK(socket_cpu_kernel_dispatch);
    

}