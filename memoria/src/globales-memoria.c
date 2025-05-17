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
t_contexto* nuevo_contexto_provisorio; 
t_contexto* nuevo_contexto;
int fd_escucha_servidor;