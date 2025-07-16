#include "instruccion.h"


void syscall_IO(char** parte){
    log_instruccion(parte);
    t_paquete* paquete = crear_super_paquete(IO);
    cargar_uint32_t_al_super_paquete(paquete, contexto->pid);
    cargar_uint32_t_al_super_paquete(paquete, contexto->registros.PC);
    cargar_string_al_super_paquete(paquete,parte[1]);
    log_debug(logger_cpu,"Antes de strtoll");
    int64_t tiempo = (int64_t) strtoll(parte[2], NULL, 10);
    log_debug(logger_cpu,"despues de strtoll");
    cargar_int64_t_al_super_paquete(paquete,tiempo);
    enviar_paquete(paquete,socket_cpu_kernel_dispatch);
    log_debug(logger_cpu,"Mando paquete syscall IO");
    eliminar_paquete(paquete);
    string_array_destroy(parte);
    marcarInterrupcionSincronica();
}

void syscallINIT_PROC(char** parte)
{
    log_instruccion(parte);
    t_paquete* paquete = crear_super_paquete(INIT_PROCCESS);
    cargar_uint32_t_al_super_paquete(paquete, contexto->pid);
    cargar_string_al_super_paquete(paquete, parte[1]);
    cargar_uint32_t_al_super_paquete(paquete,(uint32_t) strtoul(parte[2], NULL, 10));
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    string_array_destroy(parte);
}

void syscallDUMP_MEMORY(char** parte)
{
    log_instruccion(parte);
    t_paquete* paquete = crear_super_paquete(DUMP_MEMORY);
    cargar_uint32_t_al_super_paquete(paquete, contexto->pid);
    cargar_uint32_t_al_super_paquete(paquete, contexto->registros.PC);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    string_array_destroy(parte);
    marcarInterrupcionSincronica();
}

void syscallEXIT(char** parte)
{
    log_instruccion(parte);
    t_paquete* paquete = crear_super_paquete(SYSCALL_EXIT);
    cargar_uint32_t_al_super_paquete(paquete, contexto->pid);
    cargar_uint32_t_al_super_paquete(paquete, contexto->registros.PC);
    enviar_paquete(paquete, socket_cpu_kernel_dispatch);
    eliminar_paquete(paquete);
    string_array_destroy(parte);
    marcarInterrupcionSincronica();
}

void marcarInterrupcionSincronica()
{
    sem_wait(&mutex_motivo_interrupcion);
        if(flag_interrupcion == true)
        {
            sem_post(&mutex_motivo_interrupcion);
            return; //para no sobreescribir si ya llego una asincronica
        }

        flag_interrupcion = true;
        motivo_interrupcion = INTERRUPCION_SINCRONICA;
    sem_post(&mutex_motivo_interrupcion);
}