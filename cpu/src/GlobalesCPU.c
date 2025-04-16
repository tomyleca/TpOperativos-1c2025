#include "GlobalesCPU.h"

int socket_cpu_memoria;
int socket_cpu_kernel_dispatch;
int socket_cpu_kernel_interrupt;
int fd_cpu_kernel_dispatch;
int fd_cpu_kernel_interrupt;
char* ip;
char* puerto;
char* valor;

t_log* logger_cpu;
t_config* config_cpu;

char* ip_memoria;
int puerto_memoria;
char* ip_kernel;
int puerto_kernel_dispatch;
int puerto_kernel_interrupt;
int entradas_tlb;
char* reemplazo_tlb;
int entradas_cache;
char* reemplazo_cache;
int retardo_cache;
t_log_level log_level;