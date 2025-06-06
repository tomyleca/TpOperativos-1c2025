#include "globales-memoria.h"


int conexion;
char* ip;
char* puerto;
char* valor;

t_log* logger_memoria;
t_config* config_memoria;
int fd_escucha_servidor;

t_list* lista_contextos;
char* puerto_escucha;
int TAM_MEMORIA;
int TAM_PAGINA;
int ENTRADAS_POR_TABLA;
int CANTIDAD_NIVELES;
int CANT_FRAMES;
int retardo_memoria;
char* path_swapfile;
int retardo_swap;
t_log_level log_level;
char* dump_path;
pthread_t hilo_memoria;

Proceso* nuevo_contexto_provisorio; 
Proceso* nuevo_contexto;

int fd_escucha_servidor;