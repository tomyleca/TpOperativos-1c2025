#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include <commons/collections/list.h>
#include<readline/readline.h>
#include "globalesKernel.h" 
#include "../../utils/src/utils/configs.h"
#include"../../utils/src/utils/shared.h"
#include "cambiarDeEstado.h"
#include"../../utils/src/utils/conexiones.h"
#include <commons/temporal.h>

extern void dump_memory(uint32_t pid);
extern void* manejarProcesoEsperandoDump(ProcesoEnEsperaDump* procesoEsperandoDump);
extern void syscall_IO(uint32_t pid, char* nombreIO, int64_t tiempo);
extern void INIT_PROC(char* archivoPseudocodigo,uint32_t tam);
extern void syscallExit(uint32_t pid);



#endif