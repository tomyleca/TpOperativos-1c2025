#ifndef UTILS_SHARED_H_
#define UTILS_SHARED_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

typedef enum
{
    MENSAJE,
    PAQUETE,
    HANDSHAKE
} op_code; 

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

/**
* @brief Imprime un saludo por consola
* @param quien Módulo desde donde se llama a la función
* @return No devuelve nada
*/
void saludar(char* quien);

void terminar_programa(int conexion, t_log* logger, t_config* config);

void enviar_mensaje(char* mensaje, int socket_cliente);

void crear_buffer(t_paquete* paquete);

t_paquete* crear_paquete(void);

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);

void enviar_paquete(t_paquete* paquete, int socket_cliente);

void eliminar_paquete(t_paquete* paquete);

void liberar_conexion(int socket_cliente);

void *serializar_paquete(t_paquete *paquete, int bytes);

void recibir_mensaje(int socket_cliente, t_log *logger);

void *recibir_buffer(int *size, int socket_cliente);

int recibir_operacion(int socket_cliente);

t_list *recibir_paquete(int socket_cliente);




#endif
