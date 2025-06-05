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

