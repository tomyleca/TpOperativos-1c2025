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
// Variables globales externas
// =====================

extern bool *bitmap_frames;
extern void *memoria_principal;
extern t_diccionarioConSemaforos* diccionarioProcesos;
extern t_list* tabla_swap;

// =====================
// Prototipos
// =====================
//* Configuración e Inicialización
void leerConfigMemoria(t_config* config_memoria);
void inicializar_memoria();

//* Gestión de memoria 
int reservar_memoria(Proceso *p, int bytes);
void liberar_memoria(Proceso *p);

//* Gestión de tablas de páginas y frames
TablaPagina *crear_tabla_nivel(int nivel_actual);
void liberar_tabla(TablaPagina *tabla);
void asignar_frames_hojas(TablaPagina *tabla);
void imprimir_tabla(TablaPagina *tabla, int nivel, int indent);
int *reservar_frames(int cantidad);
int asignar_frame_libre();
void mostrar_bitmap();

//* Dump de memoria
void dump_memory (Proceso *p);
bool realizar_dump_memoria(int pid);

//*Operaciones sobre la memoria
int escribir_memoria(Proceso *p, int dir_fisica, char* valor);
char* leer_memoria(Proceso *p, int dir_fisica,int tamanio);

//*Gestión de procesos
Proceso* guardarProceso(uint32_t PID,uint32_t tam, char* pseudocodigo);
int guardarProcesoYReservar(uint32_t PID,uint32_t tam, char* pseudocodigo);
void destruir_proceso(uint32_t pid);
void mostrar_procesos_activos();

//*Swap
int suspender_proceso(Proceso* p, int dir_fisica);
int restaurar_proceso(Proceso* p);
void liberar_entrada_swap(int pid) ;

#endif

