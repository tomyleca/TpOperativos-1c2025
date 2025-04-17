#ifndef CONEXION_CPU_MEMORIA_H_
#define CONEXION_CPU_MEMORIA_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <pthread.h>
#include "utils/shared.h"
#include "instruccion.h"
#include "GlobalesCPU.h"


pthread_t escuchar_memoria();
pthread_t escuchar_kernel();
pthread_t escuchar_interrupcion_kernel();
pthread_t crear_hilo_interpretar_instruccion();

void atender_memoria();
void atender_interrupcion_kernel();
void atender_dispatch_kernel();


#endif