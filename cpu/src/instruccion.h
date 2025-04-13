#ifndef INSTRUCCION_H_
#define INSTRUCCION_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include "utils/shared.h"

typedef enum
{
	I_READ_MEM,
	I_WRITE_MEM,
    I_DUMP_MEMORY,
    I_IO
}enum_instrucciones;


typedef struct 
{
    int pid;
    t_registros registros;
} t_contexto_cpu;

int obtener_valor_registro_segun_nombre(char* nombre_registro);

void interpretar_instruccion_de_memoria();

void iniciar_diccionario_registros(t_registros registros);

void destruir_diccionarios(t_registros registros, enum_instrucciones instrucciones);

void iniciar_diccionario_instrucciones(enum_instrucciones instrucciones);

void solicitar_contexto_a_memoria();

void cargar_registros(t_buffer* buffer);

void cargar_registros_a_paquete(t_buffer* buffer_memoria);

void peticion_escritura_a_memoria(int direccion_fisica, int valor_registro_dato);

void instruccion_escribir_memoria(char** parte);



//-----------CICLO DE INSTRUCCION---------------------

void fetch(int socket_cpu_memoria);

void decode();

void check_interrupt();


#endif