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
#include "swap.h"

// =====================
// Configuración de memoria
// =====================

extern bool *bitmap_frames;
extern char *memoria_real;

// =====================
// Estructuras de datos
// =====================


extern t_diccionarioConSemaforos* diccionarioProcesos;

extern int cantidad_Procesos;

typedef struct {
    MetricaProceso metricas;
    uint32_t ultimo_acceso;    // Para LRU
    bool referencia;           // Para CLOCK
    bool modificada;           // Para CLOCK-M
} t_metricas_extendidas;

// =====================
// Prototipos
// =====================
void leerConfigMemoria(t_config* config_memoria);
void inicializar_memoria();
TablaPagina *crear_tabla_nivel(int nivel_actual);
void imprimir_tabla(TablaPagina *tabla, int nivel, int indent);
void liberar_tabla(TablaPagina *tabla);
void mostrar_bitmap();
int asignar_frame_libre();
void asignar_frames_hojas(TablaPagina *tabla);

//* interaccion con memoria
Proceso* guardarProceso(uint32_t PID,uint32_t tam, char* pseudocodigo);
int *reservar_frames(int cantidad);
int reservar_memoria(Proceso *p, int bytes);
void escribir_byte(Proceso *p, int direccion_virtual, char valor);
void escribir_memoria(Proceso *p, int direccion_virtual, char valor);
char leer_byte(Proceso *p, int direccion_virtual);
void leer_memoria(Proceso *p, int direccion_virtual);
Proceso *guardarProcesoYReservar(uint32_t PID,uint32_t tam, char* pseudocodigo);
void mostrar_procesos_activos();
void liberar_memoria(Proceso *p);
void destruir_proceso(Proceso *p);
char **leer_instrucciones(const char *ruta, int *cantidad);
void interpretar_instruccion(char *linea);

void dump_memory (Proceso *p);
void actualizar_metricas(Proceso* p, int tipo_operacion);
void generar_dump_memoria(Proceso* p);
void log_metricas_proceso(Proceso* p);
// Constantes para tipos de operaciones
#define ACCESO_TABLA_PAGINAS 1
#define INSTRUCCION_SOLICITADA 2
#define BAJADA_SWAP 3
#define SUBIDA_MEMORIA 4
#define LECTURA_MEMORIA 5
#define ESCRITURA_MEMORIA 6

#endif

