#ifndef CONEXIONES_KERNEL_H_
#define CONEXIONES_KERNEL_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <pthread.h>
#include "utils/shared.h"
#include "kernel.h"
#include "utils/conexiones.h"
#include "utils/configs.h"

extern pthread_t escuchar_dispatch_cpu();

extern pthread_t escuchar_interrupcion_cpu();

extern pthread_t crear_hilo_memoria();

extern void hilo_memoria();

extern void atender_dispatch_cpu();

extern void atender_interrupcion_cpu();

extern void ejecutar_io();

extern void enviar_pid_contexto_cpu(int socket_kernel_cpu_dispatch);

#endif