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

void dump_memory(uint32_t pid);
void syscall_io(uint32_t pid, char* nombreIO, uint32_t tiempo);

PCB* buscarPCBEnLista(t_list* lista, uint32_t pid);

#endif