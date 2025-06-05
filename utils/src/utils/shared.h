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
    OK,
    ERROR,
    MENSAJE,
    PAQUETE,
    HANDSHAKE,
    HANDSHAKE_CPU_KERNEL_I,
    HANDSHAKE_CPU_KERNEL_D,
    IO,
    DUMP_MEMORY,
    INIT_PROCCESS,
    SYSCALL_EXIT,
    SEGMENTATION_FAULT,
    CPU_PIDE_INSTRUCCION_A_MEMORIA,
    ENVIAR_A_MEMORIA_UN_AVISO_DE_SYSCALL,
    INTERRUPCION_PID,
    PID_KERNEL_A_CPU,
    CPU_RECIBE_INSTRUCCION_MEMORIA,
    CPU_RECIBE_CONTEXTO,
    CPU_RECIBE_OK_DE_LECTURA,
    CPU_RECIBE_OK_DE_ESCRITURA,
    CPU_PIDE_ESCRIBIR_MEMORIA,
    CPU_PIDE_LEER_MEMORIA,
    CPU_PIDE_CONTEXTO,
    GUARDAR_PROCESO_EN_MEMORIA,
    RESPUESTA_KERNEL_TPN,
    CREAR_PID_OK,
    
    NO_HAY_MEMORIA

    
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

typedef struct {
    uint32_t PC;
} t_registros;

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

void liberar_array_strings(char **array);

void *serializar_paquete(t_paquete *paquete, int bytes);

void recibir_mensaje(int socket_cliente, t_log *logger);

void *recibir_buffer(int *size, int socket_cliente);

int recibir_operacion(int socket_cliente);

t_list *recibir_paquete(int socket_cliente);

t_paquete *crear_super_paquete(op_code code_op);

void cargar_string_al_super_paquete(t_paquete *paquete, char *string);

void cargar_uint32_t_al_super_paquete(t_paquete *paquete, uint32_t numero);

void cargar_int_al_super_paquete(t_paquete *paquete, int numero);

t_buffer *recibiendo_super_paquete(int conexion);

void *recibir_cosas_del_buffer(t_buffer *coso);

void cargar_cosas_al_super_paquete(t_paquete *paquete, void *choclo, int size);

uint32_t recibir_uint32_t_del_buffer(t_buffer *coso);

int64_t recibir_int64_t_del_buffer(t_buffer *coso);

int recibir_int_del_buffer(t_buffer *coso);

char *recibir_string_del_buffer(t_buffer *palabra);




#endif
