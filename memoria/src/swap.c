#include "swap.h"
#include "globales-memoria.h"
#include "utils/configs.h"
#include <commons/log.h>
#include <string.h>
#include <pthread.h>

// Variable global del sistema de SWAP
t_swap* swap_system;

// Implementación de funciones auxiliares
void escribir_pagina_en_swap(TablaPagina* tabla, int nivel, Proceso* p) {
    if (tabla == NULL) return;
    
    if (tabla->es_hoja) {
        for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
            if (tabla->frames[i] != -1) {
                // Obtener el contenido de la página
                void* contenido = malloc(TAM_PAGINA);
                if (contenido == NULL) {
                    log_error(logger_memoria, "Error al asignar memoria para contenido de página");
                    continue;
                }
                
                memcpy(contenido, memoria_real + (tabla->frames[i] * TAM_PAGINA), TAM_PAGINA);
                
                // Escribir en SWAP
                if (escribir_pagina_swap(p->pid, i, contenido) == 0) {
                    // Liberar el frame
                    bitmap_frames[tabla->frames[i]] = false;
                    tabla->frames[i] = -1;
                    actualizar_metricas(p, BAJADA_SWAP);
                }
                
                free(contenido);
            }
        }
    } else {
        for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
            if (tabla->entradas[i] != NULL) {
                escribir_pagina_en_swap(tabla->entradas[i], nivel + 1, p);
            }
        }
    }
}

void leer_pagina_de_swap(TablaPagina* tabla, int nivel, Proceso* p) {
    if (tabla == NULL) return;
    
    if (tabla->es_hoja) {
        for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
            // Intentar leer la página del SWAP
            void* buffer = malloc(TAM_PAGINA);
            if (buffer == NULL) {
                log_error(logger_memoria, "Error al asignar memoria para buffer de página");
                continue;
            }
            
            if (leer_pagina_swap(p->pid, i, buffer) == 0) {
                // Asignar un nuevo frame
                int frame = asignar_frame_libre();
                if (frame != -1) {
                    tabla->frames[i] = frame;
                    // Copiar el contenido a memoria
                    memcpy(memoria_real + (frame * TAM_PAGINA), buffer, TAM_PAGINA);
                    actualizar_metricas(p, SUBIDA_MEMORIA);
                }
            }
            
            free(buffer);
        }
    } else {
        for (int i = 0; i < ENTRADAS_POR_TABLA; i++) {
            if (tabla->entradas[i] != NULL) {
                leer_pagina_de_swap(tabla->entradas[i], nivel + 1, p);
            }
        }
    }
}

// Implementación de funciones principales
void inicializar_swap(void) {
    swap_system = malloc(sizeof(t_swap));
    if (swap_system == NULL) {
        log_error(logger_memoria, "Error al asignar memoria para el sistema SWAP");
        exit(1);
    }
    
    swap_system->entradas = list_create();
    if (swap_system->entradas == NULL) {
        log_error(logger_memoria, "Error al crear lista de entradas SWAP");
        free(swap_system);
        exit(1);
    }
    
    swap_system->archivo = fopen(path_swapfile, "wb+");
    if (swap_system->archivo == NULL) {
        log_error(logger_memoria, "Error al abrir el archivo de SWAP: %s", path_swapfile);
        list_destroy(swap_system->entradas);
        free(swap_system);
        exit(1);
    }
    
    if (pthread_mutex_init(&swap_system->mutex, NULL) != 0) {
        log_error(logger_memoria, "Error al inicializar mutex de SWAP");
        fclose(swap_system->archivo);
        list_destroy(swap_system->entradas);
        free(swap_system);
        exit(1);
    }
    
    log_info(logger_memoria, "Sistema de SWAP inicializado correctamente");
}

void cerrar_swap(void) {
    if (swap_system != NULL) {
        pthread_mutex_destroy(&swap_system->mutex);
        list_destroy_and_destroy_elements(swap_system->entradas, free);
        fclose(swap_system->archivo);
        free(swap_system);
        swap_system = NULL;
    }
}

int escribir_pagina_swap(uint32_t pid, uint32_t pagina, void* contenido) {
    if (swap_system == NULL || contenido == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&swap_system->mutex);
    
    // Buscar si ya existe una entrada para esta página
    t_entrada_swap* entrada = NULL;
    for (int i = 0; i < list_size(swap_system->entradas); i++) {
        t_entrada_swap* elem = list_get(swap_system->entradas, i);
        if (buscar_entrada_swap(elem, pid, pagina)) {
            entrada = elem;
            break;
        }
    }
    
    if (entrada == NULL) {
        // Crear nueva entrada
        entrada = malloc(sizeof(t_entrada_swap));
        if (entrada == NULL) {
            pthread_mutex_unlock(&swap_system->mutex);
            return -1;
        }
        
        entrada->pid = pid;
        entrada->pagina = pagina;
        entrada->offset_swap = list_size(swap_system->entradas) * TAM_PAGINA;
        entrada->modificada = false;
        list_add(swap_system->entradas, entrada);
    }
    
    // Escribir la página en el archivo
    fseek(swap_system->archivo, entrada->offset_swap, SEEK_SET);
    size_t escritos = fwrite(contenido, 1, TAM_PAGINA, swap_system->archivo);
    fflush(swap_system->archivo);
    
    pthread_mutex_unlock(&swap_system->mutex);
    return (escritos == TAM_PAGINA) ? 0 : -1;
}

int leer_pagina_swap(uint32_t pid, uint32_t pagina, void* buffer) {
    if (swap_system == NULL || buffer == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&swap_system->mutex);
    
    // Buscar la entrada correspondiente
    t_entrada_swap* entrada = NULL;
    for (int i = 0; i < list_size(swap_system->entradas); i++) {
        t_entrada_swap* elem = list_get(swap_system->entradas, i);
        if (buscar_entrada_swap(elem, pid, pagina)) {
            entrada = elem;
            break;
        }
    }
    
    if (entrada == NULL) {
        pthread_mutex_unlock(&swap_system->mutex);
        return -1;
    }
    
    // Leer la página del archivo
    fseek(swap_system->archivo, entrada->offset_swap, SEEK_SET);
    size_t leidos = fread(buffer, 1, TAM_PAGINA, swap_system->archivo);
    
    pthread_mutex_unlock(&swap_system->mutex);
    return (leidos == TAM_PAGINA) ? 0 : -1;
}

void suspender_proceso(Proceso* p) {
    if (p == NULL || p->tabla_raiz == NULL) {
        return;
    }
    escribir_pagina_en_swap(p->tabla_raiz, 0, p);
    log_info(logger_memoria, "Proceso %d suspendido - Páginas movidas a SWAP", p->pid);
}

void desuspender_proceso(Proceso* p) {
    if (p == NULL || p->tabla_raiz == NULL) {
        return;
    }
    leer_pagina_de_swap(p->tabla_raiz, 0, p);
    log_info(logger_memoria, "Proceso %d desuspendido - Páginas cargadas de SWAP", p->pid);
}
