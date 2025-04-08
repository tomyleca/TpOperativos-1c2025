#ifndef UTILS_CONEXIONES_H_
#define UTILS_CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include "shared.h"

// CLIENTE
int crear_conexion(t_log* server_name, char* ip, char* puerto);
void generar_handshake(int socket, char *server_name, char *ip, char *puerto);

//SERVIDOR

int esperar_cliente(int socket_servidor);
int iniciar_servidor(t_log* logger, char *puerto);
void recibir_handshake(int socket);


//void realizar_handshake(op_code module, int server);

#endif