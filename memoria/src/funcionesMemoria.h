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
#include "GlobalesMemoria.h"

t_list* leer_archivo_y_cargar_instrucciones(char* archivo_pseudocodigo);

void crear_pid(t_contexto* nuevo_contexto, t_info_kernel* info_kernel);

t_contexto* buscar_contexto_por_pid(int pid);


#endif