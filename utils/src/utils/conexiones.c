#include "conexiones.h"


//-----------------------------------------------CLIENTE----------------------------------------------
int crear_conexion(t_log* nombreLogger, char* ip, char* puerto) {
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int resultado = getaddrinfo(ip, puerto, &hints, &servinfo);
    if (resultado != 0) {
        log_error(nombreLogger, "getaddrinfo fallo: %s", gai_strerror(resultado));
        free(puerto);
        return -1;
    }

    int socket_cliente = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (socket_cliente == -1) {
        log_error(nombreLogger, "Error creando socket");
        freeaddrinfo(servinfo);
        free(puerto);
        return -1;
    }

    if (connect(socket_cliente, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        log_error(nombreLogger, "Error al conectar con %s:%s", ip, puerto);
        freeaddrinfo(servinfo);
        free(puerto);
        exit(1);
    }

    log_info(nombreLogger, "Conexion exitosa con %s:%s", ip, puerto);

    freeaddrinfo(servinfo);
    return socket_cliente;
}



void generar_handshake(int socket, char *nombreLogger, char *ip, char *puerto) {
    int32_t handshake = 1;
    int32_t result;

	op_code cod_op = HANDSHAKE; 
	send(socket, &cod_op, sizeof(op_code), 0);
    
    send(socket, &handshake, sizeof(int32_t), 0);
	recv(socket, &result, sizeof(int32_t), MSG_WAITALL);

}
//----------------------------------------SERVIDOR------------------------------------------------------------------
int iniciar_servidor(t_log* logger, char* puerto)
{
	int fd_servidor;
    struct addrinfo hints, *servinfo;
	//struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

    int resultado = getaddrinfo(NULL, puerto, &hints, &servinfo);
    if (resultado != 0) {
        log_error(logger, "Error en getaddrinfo: %s\n", gai_strerror(resultado));
        exit(EXIT_FAILURE);
    }

    fd_servidor = socket(servinfo->ai_family,
                        servinfo->ai_socktype,
                        servinfo->ai_protocol);
    if(fd_servidor == -1) {
        log_error(logger, "Error creando el socket del servidor\n");
        exit(EXIT_FAILURE);
    }
    
    setsockopt(fd_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

	//Escucha y bindeo a puerto
	bind(fd_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	if (listen(fd_servidor, SOMAXCONN)==-1)
	{
		log_error(logger,"Hubo un error al intentar comenzar la escucha");
		exit(EXIT_FAILURE);
		return -1;
	}

	//printf("[ INFO ]: << SERVIDOR LISTO Y ESCUCHANDO	>>\n");

	freeaddrinfo(servinfo);

	return fd_servidor;
}


int esperar_cliente(int socket_servidor) {
    struct sockaddr_in dir_cliente;
    socklen_t tam_direccion = sizeof(struct sockaddr_in);

    int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);


    return socket_cliente;
}

void enviarOpCode(int fdConexion, op_code opCode)
{
    op_code codigo = opCode;
    send(fdConexion,&codigo,sizeof(int),0);
}

void enviarOK(int fdConexion)
{
    if (fdConexion <= 0) {
        return;
    }
    
    int OK = 1;
    send(fdConexion, &OK, sizeof(int), 0);
}

bool esperarOK(int fdConexion)
{
    int OK;
    return recv(fdConexion,&OK,sizeof(int),0);
}

