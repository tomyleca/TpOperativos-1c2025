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
#include "globalesKernel.h"
#include "utils/conexiones.h"
#include "utils/configs.h"
#include "conexionConCPU.h"

pthread_t escuchar_dispatch_cpu();

pthread_t escuchar_interrupcion_cpu();

pthread_t crear_hilo_memoria();

void hilo_memoria();

void atender_dispatch_cpu();

void atender_interrupcion_cpu();

void ejecutar_io();

void enviar_pid_contexto_cpu(int socket_kernel_cpu_dispatch);

#endif