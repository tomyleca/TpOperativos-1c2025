#ifndef SWAP_H_
#define SWAP_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include "administracion-memoria.h"

// Estructura para manejar el SWAP
typedef struct {
    uint32_t pid;
    uint32_t pagina;
    uint32_t offset_swap;  // Posición en el archivo swap
    bool modificada;       // Flag para CLOCK-M
} t_entrada_swap;

// Estructura para el archivo SWAP
typedef struct {
    FILE* archivo;
    t_list* entradas;     // Lista de t_entrada_swap
    pthread_mutex_t mutex;
} t_swap;

// Variables globales
extern t_swap* swap_system;
extern char* path_swapfile;
extern int tam_pagina;
extern int entradas_por_tabla;
extern char* memoria_real;
extern bool* bitmap_frames;
extern t_log* logger_memoria;

// Funciones auxiliares
static inline bool buscar_entrada_swap(void* elem, uint32_t pid, uint32_t pagina) {
    t_entrada_swap* entrada = (t_entrada_swap*)elem;
    return entrada->pid == pid && entrada->pagina == pagina;
}

// Prototipos de funciones
void inicializar_swap(void);
void cerrar_swap(void);
int escribir_pagina_swap(uint32_t pid, uint32_t pagina, void* contenido);
int leer_pagina_swap(uint32_t pid, uint32_t pagina, void* buffer);
void suspender_proceso(Proceso* p);
void desuspender_proceso(Proceso* p);

// Funciones auxiliares para suspender/desuspender
void escribir_pagina_en_swap(TablaPagina* tabla, int nivel, Proceso* p);
void leer_pagina_de_swap(TablaPagina* tabla, int nivel, Proceso* p);

#endif /* SWAP_H_ */

