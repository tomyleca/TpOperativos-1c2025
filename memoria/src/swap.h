#ifndef ADMINISTRACION_MEMORIA_H_
#define ADMINISTRACION_MEMORIA_H_


#include "globales-memoria.h"
#include "utils/conexiones.h"
#include "utils/configs.h"
#include "utils/shared.h"
#include "utils/monitoresDiccionarios.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <funciones-memoria.h>
#include <globales-memoria.h>

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
#endif

