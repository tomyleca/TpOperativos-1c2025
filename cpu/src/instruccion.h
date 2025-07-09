#ifndef INSTRUCCION_H_
#define INSTRUCCION_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <math.h>
#include "utils/shared.h"
#include "GlobalesCPU.h"

typedef enum
{
    I_WRITE_MEM,
	I_READ_MEM,
    I_DUMP_MEMORY,
    I_IO,
    I_NOOP,
    I_GOTO,
    I_INIT_PROCESS,
    I_EXIT
}enum_instrucciones;

void iniciar_diccionario_instrucciones();

void destruir_diccionarios();

void solicitar_contexto_a_memoria(t_contexto_cpu* contexto);

void enviar_interrupcion_a_kernel_y_memoria(char** instruccion, op_code motivo_de_interrupcion);


//--------------INSTRUCCIONES-----------------//

void instruccion_noop();

void instruccion_escribir_memoria();

void instruccion_leer_memoria();

void instruccion_goto();

//-------------SYSCALLS --------------//

void syscall_IO(char** parte);

void syscallDUMP_MEMORY(char** parte);

void syscallEXIT(char** parte);

void syscallINIT_PROC(char** parte);

void log_instruccion(char** parte);

//-------------SYSCALLS MEMORIA FUNCIONES--------------//

void peticion_lectura_a_memoria(int direccion_fisica, int tamanio);

void peticion_escritura_a_memoria(int direccion_fisica, char* valor_registro_dato);

void instruccion_escribir_memoria(char** parte);

//-----------CICLO DE INSTRUCCION---------------------//

void ciclo_instruccion(int socket_cpu_memoria);

void fetch(int socket_cpu_memoria);

void decode();

void check_interrupt(uint32_t PIDInicial);


//---------MMU--------------//

int traducirDLMedianteMMU(int nro_pagina);

void liberar_entradas_tlb();

void destruir_entrada_tlb(void* entrada_tlb);

int traducir_direccion_logica(int direccion_logica);

int buscar_en_tlb(int direccion_logica);

void agregar_a_tlb(int pid, int nro_pagina, int nro_marco);

int obtener_timestamp_actual();

//---------CACHE--------------//

int cargar_pagina_en_cache(int pid, int direccion_logica);

void escribir_cache(int direccion_logica, char* valor);

void inicializar_cache();

int buscar_en_cache(int pid, int nro_pagina);

void leer_cache(int direccion_fisica, int tamanio);

int algoritmo_clock();

int algoritmo_clock_modificado();

int seleccionar_victima();

void solicitar_marco_a_memoria(int* entradas_de_nivel);

void solicitar_tabla_a_memoria();

void desalojar_proceso_de_cache();

void liberar_cache();


#endif