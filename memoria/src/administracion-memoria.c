#include "administracion-memoria.h"

// Definición de las variables globales
TablaNivel** tablas_nivel = NULL; // Inicializa como NULL
int cantidad_tablas = 0;    
void* memoria_usuario = NULL; 

void leerConfigMemoria(t_config* config_memoria) 
{
    puerto_escucha = config_get_int_value(config_memoria, "PUERTO_ESCUCHA");
    tam_memoria = config_get_int_value(config_memoria, "TAM_MEMORIA");
    tam_pagina = config_get_int_value(config_memoria, "TAM_PAGINA");
    entradas_por_tabla = config_get_int_value(config_memoria, "ENTRADAS_POR_TABLA");
    cant_niveles = config_get_int_value(config_memoria, "CANTIDAD_NIVELES");
    retardo_memoria = config_get_int_value(config_memoria, "RETARDO_MEMORIA");
    path_swapfile = config_get_string_value(config_memoria, "PATH_SWAPFILE");
    retardo_swap = config_get_int_value(config_memoria, "RETARDO_SWAP");
    log_level = log_level_from_string(config_get_string_value(config_memoria, "LOG_LEVEL"));
    dump_path = config_get_string_value(config_memoria, "DUMP_PATH"); 
}

int asignar_pagina(int nivel, int entrada, int valor) {
    if (nivel >= cant_niveles || entrada >= entradas_por_tabla) {
        log_error(logger_memoria, "Error: Índice fuera de rango en la tabla de nivel.");
        return -1;
    }
    tablas_nivel[nivel]->entradas[entrada] = valor;
    return 0;
}

void liberar_pagina(int nivel, int entrada) {
    if (nivel < cant_niveles && entrada < entradas_por_tabla) {
        tablas_nivel[nivel]->entradas[entrada] = -1; // Liberar la entrada
    }
}


void escribir_memoria(int direccion_fisica, void* dato, size_t tamanio) {
    if (direccion_fisica + tamanio > tam_memoria) {
        log_error(logger_memoria, "Error: Escritura fuera de los límites de la memoria.");
        return;
    }
    memcpy(memoria_usuario + direccion_fisica, dato, tamanio);
}

void leer_memoria(int direccion_fisica, void* buffer, size_t tamanio) {
    if (direccion_fisica + tamanio > tam_memoria) {
        log_error(logger_memoria, "Error: Lectura fuera de los límites de la memoria.");
        return;
    }
    memcpy(buffer, memoria_usuario + direccion_fisica, tamanio);
}

// Inicializar las tablas de páginas jerárquicas
void inicializar_tablas_paginacion() {
    cantidad_tablas = pow(entradas_por_tabla, cant_niveles); // Total de tablas necesarias
    tablas_nivel = malloc(cantidad_tablas * sizeof(TablaNivel*));

    for (int i = 0; i < cantidad_tablas; i++) {
        tablas_nivel[i] = malloc(sizeof(TablaNivel));
        tablas_nivel[i]->entradas = malloc(entradas_por_tabla * sizeof(int));
        for (int j = 0; j < entradas_por_tabla; j++) {
            tablas_nivel[i]->entradas[j] = -1; // Inicializar con -1 (sin asignar)
        }
    }
}
int traducir_direccion_logica(int direccion_logica) {
    int desplazamiento = direccion_logica % tam_pagina;
    int indice_tabla = direccion_logica / tam_pagina;

    for (int nivel = 0; nivel < cant_niveles; nivel++) {
        int entrada = (indice_tabla / (int)pow(entradas_por_tabla, cant_niveles - nivel - 1)) % entradas_por_tabla;
        indice_tabla = tablas_nivel[nivel]->entradas[entrada];
        if (indice_tabla == -1) {
            log_error(logger_memoria, "Error: Dirección lógica no asignada.");
            return -1;
        }
    }

    return indice_tabla * tam_pagina + desplazamiento;
}

void inicializar_estructuras_memoria()
{
    memoria_usuario = malloc(tam_memoria); 
    if (memoria_usuario == NULL) {
         log_error(logger_memoria, "No se pudo reservar memoria para la memoria de usuario");
         exit(EXIT_FAILURE);
    }

    memset(memoria_usuario, 0, tam_memoria); // Inicializar con ceros

    inicializar_tablas_paginacion();
    log_info(logger_memoria, "Se inicializaron las tablas de paginacion");

}