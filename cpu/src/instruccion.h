#ifndef INSTRUCCION_H_
#define INSTRUCCION_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils/shared.h"
#include "GlobalesCPU.h"

typedef enum
{
	I_READ_MEM,
	I_WRITE_MEM,
    I_DUMP_MEMORY,
    I_IO,
    I_NOOP,
    I_GOTO
}enum_instrucciones;



int obtener_valor_registro_segun_nombre(char* nombre_registro);

void iniciar_diccionario_registros();

void destruir_diccionarios();

void iniciar_diccionario_instrucciones();

void solicitar_contexto_a_memoria(t_contexto_cpu* contexto);

void cargar_registros(t_buffer* buffer);

void cargar_registros_a_paquete(t_paquete* buffer_memoria);

void enviar_interrupcion_a_kernel_y_memoria(char** instruccion, op_code motivo_de_interrupcion);



//--------------INSTRUCCIONES-----------------//

void instruccion_noop();

void instruccion_escribir_memoria();

void instruccion_leer_memoria();

void instruccion_goto();

//-------------SYSCALLS MEMORIA FUNCIONES--------------//

void peticion_lectura_a_memoria(int direccion_fisica, int tamanio);

void peticion_escritura_a_memoria(int direccion_fisica, char* valor_registro_dato);

void instruccion_escribir_memoria(char** parte);

//-----------CICLO DE INSTRUCCION---------------------//

void fetch(int socket_cpu_memoria);

void decode();

void check_interrupt();

int mmu_traducir_direccion_logica(int direccion_logica);

#endif