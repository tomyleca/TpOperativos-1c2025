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
    INIT_PROCCESS,
    DUMP_MEMORY,
    SYSCALL_EXIT,
    SEGMENTATION_FAULT,
    CPU_PIDE_INSTRUCCION_A_MEMORIA,
    ENVIAR_A_MEMORIA_UN_AVISO_DE_SYSCALL,
    INTERRUPCION_SINCRONICA,
    INTERRUPCION_ASINCRONICA,
    PC_INTERRUPCION_ASINCRONICA,
    PID_KERNEL_A_CPU,
    CPU_RECIBE_INSTRUCCION_MEMORIA,
    SOLICITUD_ESTRUCTURA_MEMORIA,
    RESPUESTA_ESTRUCTURA_MEMORIA,
    CPU_RECIBE_CONTEXTO,
    CPU_RECIBE_OK_DE_LECTURA,
    CPU_RECIBE_OK_DE_ESCRITURA,
    CPU_PIDE_ESCRIBIR_MEMORIA,
    CPU_PIDE_LEER_MEMORIA,
    CPU_PIDE_CONTEXTO,
    GUARDAR_PROCESO_EN_MEMORIA,
    RESPUESTA_KERNEL_TPN,
    CREAR_PID_OK,
    SOLICITUD_TABLA,
    SOLICITUD_FRAME,
    RESPUESTA_SOLICITUD_TABLA,
    RESPUESTA_SOLICITUD_FRAME,
    NO_HAY_MEMORIA,
    SWAP_RESTAURAR_PROCESO,
    SWAP_SUSPENDER_PROCESO,
    SWAP_OK,
<<<<<<< HEAD
    CPU_SOLICITA_ESCRIBIR_PAGINA_COMPLETA,
    CPU_SOLICITA_LEER_PAGINA_COMPLETA,
=======
    SWAP_ERROR,
    CPU_LEE_PAGINA_COMPLETA,
    CPU_ESCRIBE_PAGINA_COMPLETA,
>>>>>>> f689fb49ec6e6d219c54a80196ebfabf254fc99e
    DUMP_MEMORY_OK,
    DUMP_MEMORY_ERROR,
    RESPUESTA_PAGINA_COMPLETA_CPU, 
    RESPUESTA_VALOR_LEIDO_CPU     


    
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

int enviar_todo(int socket, void* buffer, int total_bytes);

void eliminar_paquete(t_paquete* paquete);

void liberar_conexion(int socket_cliente);

void liberar_array_strings(char **array);

void *serializar_paquete(t_paquete *paquete, int bytes);

void recibir_mensaje(int socket_cliente, t_log *logger);

void *recibir_buffer(int *size, int socket_cliente);

void limpiarBuffer(t_buffer* buffer);

int recibir_operacion(int socket_cliente);

t_list *recibir_paquete(int socket_cliente);

t_paquete *crear_super_paquete(op_code code_op);

void cargar_string_al_super_paquete(t_paquete *paquete, char *string);

void cargar_uint32_t_al_super_paquete(t_paquete *paquete, uint32_t numero);

void cargar_int64_t_al_super_paquete(t_paquete* paquete,int64_t numero);

void cargar_int_al_super_paquete(t_paquete *paquete, int numero);

t_buffer *recibiendo_super_paquete(int conexion);

void *recibir_cosas_del_buffer(t_buffer *coso);

void cargar_cosas_al_super_paquete(t_paquete *paquete, void *choclo, int size);

uint32_t recibir_uint32_t_del_buffer(t_buffer *coso);

int64_t recibir_int64_t_del_buffer(t_buffer *coso);

int recibir_int_del_buffer(t_buffer *coso);

char *recibir_string_del_buffer(t_buffer *palabra);




#endif
