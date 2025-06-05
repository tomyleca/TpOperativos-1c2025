#ifndef SWAP_H_
#define SWAP_H_

#include <stdint.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include "administracion-memoria.h"

typedef struct {
    uint32_t pid;
    uint32_t pagina;
    uint32_t offset_swap;  // Posición en el archivo swap
    bool modificada;       // Flag para CLOCK-M
} t_entrada_swap;

typedef struct {
    FILE* archivo;
    t_list* entradas;     // Lista de t_entrada_swap
    pthread_mutex_t mutex;
} t_swap;

extern t_swap* swap_system;

void inicializar_swap();
void cerrar_swap();
int escribir_pagina_swap(uint32_t pid, uint32_t pagina, void* contenido);
int leer_pagina_swap(uint32_t pid, uint32_t pagina, void* buffer);
void suspender_proceso(Proceso* p);
void desuspender_proceso(Proceso* p);

#endif

