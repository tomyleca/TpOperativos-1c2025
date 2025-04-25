#include "conexiones.h"

//-----------------------------------------------CLIENTE----------------------------------------------
int crear_conexion(t_log* server_name, char* ip, int puerto) {
    struct addrinfo hints, *servinfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char* puerto_str = NULL;
    asprintf(&puerto_str, "%d", puerto);

    int resultado = getaddrinfo(ip, puerto_str, &hints, &servinfo);
    if (resultado != 0) {
        log_error(server_name, "getaddrinfo fallo: %s", gai_strerror(resultado));
        free(puerto_str);
        return -1;
    }

    int socket_cliente = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (socket_cliente == -1) {
        log_error(server_name, "Error creando socket");
        freeaddrinfo(servinfo);
        free(puerto_str);
        return -1;
    }

    if (connect(socket_cliente, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        log_error(server_name, "Error al conectar con %s:%s", ip, puerto_str);
        freeaddrinfo(servinfo);
        free(puerto_str);
        return -1;
    }

    log_info(server_name, "Conexion exitosa con %s:%s", ip, puerto_str);

    freeaddrinfo(servinfo);
    free(puerto_str);
    return socket_cliente;
}


//----------------------------------------SERVIDOR------------------------------------------------------------------
int iniciar_servidor(t_log* logger, int puerto)
{
    char puerto_str[10];
    sprintf(puerto_str, "%d", puerto);

	int fd_servidor;
    struct addrinfo hints, *servinfo;
	//struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	fd_servidor = getaddrinfo(NULL, puerto_str, &hints, &servinfo);
	fd_servidor = socket(servinfo->ai_family,
                        	servinfo->ai_socktype,
                        	servinfo->ai_protocol);

	if(fd_servidor==-1)
	{
		log_error(logger,"Hubo un error creando el file descriptor del servidor escucha\n");
		exit(EXIT_FAILURE);
		return fd_servidor;
	}

	//Escucha y bindeo a puerto
	bind(fd_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	if (listen(fd_servidor, SOMAXCONN)==-1)
	{
		log_error(logger,"Hubo un error al intentar comenzar la escucha");
		exit(EXIT_FAILURE);
		return -1;
	}

	printf("[ INFO ]: << SERVIDOR LISTO Y ESCUCHANDO	>>\n");

	freeaddrinfo(servinfo);

	return fd_servidor;
}

int esperar_cliente(int socket_servidor) {
    struct sockaddr_in dir_cliente;
    socklen_t tam_direccion = sizeof(struct sockaddr_in);

    int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);


    return socket_cliente;
}

void recibir_handshake(int socket) {
	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;

	recv(socket, &handshake, sizeof(int32_t), MSG_WAITALL);
	
	if (handshake == 1) 
		send(socket, &resultOk, sizeof(int32_t), 0);
	else 
		send(socket, &resultError, sizeof(int32_t), 0);
}

/*void realizar_handshake(op_code module, int server)
 {
     int *handshake = malloc(sizeof(int));
     if (handshake == NULL)
 	{
       log_info(logger,"Failed to allocate memory for handshake");
        return;
    }

    *handshake = module;
     ssize_t bytes_sent = send(server, handshake, sizeof(int), 0);
     if (bytes_sent == -1)
 	{
         log_info(logger,"Failed to send handshake");
     }

     free(handshake);
 }*/
