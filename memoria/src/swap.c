#include "swap.h"
#include "globales-memoria.h"
#include "utils/configs.h"
#include <commons/log.h>
#include <string.h>
#include <pthread.h>

t_swap* swap_system;

void inicializar_swap() {
    swap_system = malloc(sizeof(t_swap));
    swap_system->entradas = list_create();
    swap_system->archivo = fopen(path_swapfile, "wb+");
    pthread_mutex_init(&swap_system->mutex, NULL);
    
    if (swap_system->archivo == NULL) {
        log_error(logger_memoria, "Error al abrir el archivo de SWAP");
        exit(1);
    }
    
    log_info(logger_memoria, "Sistema de SWAP inicializado correctamente");
}

void cerrar_swap() {
    if (swap_system != NULL) {
        pthread_mutex_destroy(&swap_system->mutex);
        list_destroy_and_destroy_elements(swap_system->entradas, free);
        fclose(swap_system->archivo);
        free(swap_system);
        swap_system = NULL;
    }
}

static void liberar_entrada(void* elem) {
    free(elem);
}

int escribir_pagina_swap(uint32_t pid, uint32_t pagina, void* contenido) {
    pthread_mutex_lock(&swap_system->mutex);
    
    bool buscar_entrada(void* elem) {
        t_entrada_swap* entrada = (t_entrada_swap*)elem;
        return entrada->pid == pid && entrada->pagina == pagina;
    }
    
    t_entrada_swap* entrada = list_find(swap_system->entradas, buscar_entrada);
    
    if (entrada == NULL) {
        entrada = malloc(sizeof(t_entrada_swap));
        entrada->pid = pid;
        entrada->pagina = pagina;
        entrada->offset_swap = list_size(swap_system->entradas) * tam_pagina;
        entrada->modificada = false;
        list_add(swap_system->entradas, entrada);
    }
    
    fseek(swap_system->archivo, entrada->offset_swap, SEEK_SET);
    fwrite(contenido, 1, tam_pagina, swap_system->archivo);
    fflush(swap_system->archivo);
    
    pthread_mutex_unlock(&swap_system->mutex);
    return 0;
}

    // Buscar la entrada correspondiente
    // Leer la página del archivo
                    // Obtener el contenido de la página
                    // Escribir en SWAP
                    // Liberar el frame
                // Intentar leer la página del SWAP
                    // Asignar un nuevo frame
                        // Copiar el contenido a memoria
