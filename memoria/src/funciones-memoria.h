#ifndef FUNCIONES_MEMORIA_H_
#define FUNCIONES_MEMORIA_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include<readline/readline.h>
#include "utils/shared.h"
#include "utils/conexiones.h"
#include "utils/configs.h"
#include "globales-memoria.h"
#include "administracion-memoria.h"

t_list* leer_archivo_y_cargar_instrucciones(char* archivo_pseudocodigo);

void crear_pid(t_contexto* nuevo_contexto, t_info_kernel info_kernel);

Proceso* buscar_contexto_por_pid(int pid);

char* obtener_instruccion_por_indice(t_list* instrucciones, uint32_t indice_instruccion);

extern void buscar_y_mandar_instruccion(t_buffer *buffer, int socket_cpu);

void enviar_contexto(Proceso* contexto_proceso, int socket_cpu);


#endif