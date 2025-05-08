#ifndef ADMINISTRACION_MEMORIA_H_
#define ADMINISTRACION_MEMORIA_H_


#include "globales-memoria.h"
#include "utils/conexiones.h"
#include "utils/configs.h"
#include "utils/shared.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


typedef struct {
    int* entradas; 
} TablaNivel;

// Variables globales para las tablas de paginación
extern TablaNivel** tablas_nivel; 
extern int cantidad_tablas;

void liberar_marco(int marco) ;
int asignar_marco_libre();
void leerConfigMemoria(t_config* config_memoria);
int asignar_pagina(int nivel, int entrada, int valor);
void liberar_pagina(int nivel, int entrada);
void escribir_memoria(int direccion_fisica, void* dato, size_t tamanio);
void leer_memoria(int direccion_fisica, void* buffer, size_t tamanio);
void inicializar_tablas_paginacion();
int traducir_direccion_logica(int direccion_logica);

void inicializar_estructuras_memoria();

#endif