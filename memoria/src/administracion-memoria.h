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
int guardarProcesoYReservar(uint32_t PID,uint32_t tam, char* pseudocodigo);
void mostrar_procesos_activos();
void liberar_memoria(Proceso *p);
void destruir_proceso(Proceso *p);
char **leer_instrucciones(const char *ruta, int *cantidad);
void interpretar_instruccion(char *linea);

void dump_memory (Proceso *p);


bool crear_estructuras_proceso(int pid, int paginas_necesarias);
bool suspender_proceso(int pid);
bool des_suspender_proceso(int pid);
bool realizar_dump_memoria(int pid);
bool finalizar_proceso(int pid);

#endif

