#include <utils/shared.h>

void saludar(char* quien) {
    printf("Hola desde %s!!\n", quien);
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	log_destroy(logger);
	config_destroy(config);
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config) 
	  con las funciones de las commons y del TP mencionadas en el enunciado */
}

void *serializar_paquete(t_paquete *paquete, int bytes)
{
	void *magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}


void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int fdConexion)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(fdConexion, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

// Utils del server -----------------------------------------------------------------------------------------------------------------------



int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente, t_log *logger)
{
	int size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje: %s", buffer);
	free(buffer);
}




char *recibir_string_del_buffer(t_buffer *palabra)
{

	if (palabra->size == 0)
	{
		printf("\n[ERROR] Al intentar extraer un contenido de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if (palabra->size < 0)
	{
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int size_string;
	char *string;
	memcpy(&size_string, palabra->stream, sizeof(int));
	// string = malloc(sizeof(size_string));
	string = malloc(size_string);
	memcpy(string, palabra->stream + sizeof(int), size_string);

	int nuevo_size = palabra->size - sizeof(int) - size_string;
	if (nuevo_size == 0)
	{
		free(palabra->stream);
		palabra->stream = NULL;
		palabra->size = 0;
		return string;
	}
	if (nuevo_size < 0)
	{
		printf("\n[ERROR_STRING]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		free(string);
		// return "[ERROR]: BUFFER CON TAMAÑO NEGATIVO";
		exit(EXIT_FAILURE);
	}
	void *nuevo_palabra = malloc(nuevo_size);
	memcpy(nuevo_palabra, palabra->stream + sizeof(int) + size_string, nuevo_size);
	free(palabra->stream);
	palabra->stream = nuevo_palabra;
	palabra->size = nuevo_size;

	return string;
}

int recibir_int_del_buffer(t_buffer *coso)
{
	if (coso->size == 0)
	{
		printf("\n[ERROR] Al intentar extraer un INT de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if (coso->size < 0)
	{
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int valor_a_devolver;
	memcpy(&valor_a_devolver, coso->stream, sizeof(int));

	int nuevo_size = coso->size - sizeof(int);
	if (nuevo_size == 0)
	{
		free(coso->stream);
		coso->stream = NULL;
		coso->size = 0;
		return valor_a_devolver;
	}
	if (nuevo_size < 0)
	{
		printf("\n[ERROR_INT]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		// free(valor_a_devolver);
		// return 0;
		exit(EXIT_FAILURE);
	}
	void *nuevo_coso = malloc(nuevo_size);
	memcpy(nuevo_coso, coso->stream + sizeof(int), nuevo_size);
	free(coso->stream);
	coso->stream = nuevo_coso;
	coso->size = nuevo_size;

	return valor_a_devolver;
}


uint32_t recibir_uint32_t_del_buffer(t_buffer *coso)
{
	if (coso->size == 0)
	{
		printf("\n[ERROR] Al intentar extraer un INT de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if (coso->size < 0)
	{
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	uint32_t valor_a_devolver;
	memcpy(&valor_a_devolver, coso->stream, sizeof(uint32_t));

	uint32_t nuevo_size = coso->size - sizeof(uint32_t);
	if (nuevo_size == 0)
	{
		free(coso->stream);
		coso->stream = NULL;
		coso->size = 0;
		return valor_a_devolver;
	}
	if (nuevo_size < 0)
	{
		printf("\n[ERROR_uint32_t]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		// free(valor_a_devolver);
		// return 0;
		exit(EXIT_FAILURE);
	}
	void *nuevo_coso = malloc(nuevo_size);
	memcpy(nuevo_coso, coso->stream + sizeof(uint32_t), nuevo_size);
	free(coso->stream);
	coso->stream = nuevo_coso;
	coso->size = nuevo_size;

	return valor_a_devolver;
}



int64_t recibir_int64_t_del_buffer(t_buffer *coso)
{
	if (coso->size == 0)
	{
		printf("\n[ERROR] Al intentar extraer un INT de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if (coso->size < 0)
	{
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int64_t valor_a_devolver;
	memcpy(&valor_a_devolver, coso->stream, sizeof(int64_t));

	int64_t nuevo_size = coso->size - sizeof(int64_t);
	if (nuevo_size == 0)
	{
		free(coso->stream);
		coso->stream = NULL;
		coso->size = 0;
		return valor_a_devolver;
	}
	if (nuevo_size < 0)
	{
		printf("\n[ERROR_int64_t]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		// free(valor_a_devolver);
		// return 0;
		exit(EXIT_FAILURE);
	}
	void *nuevo_coso = malloc(nuevo_size);
	memcpy(nuevo_coso, coso->stream + sizeof(int64_t), nuevo_size);
	free(coso->stream);
	coso->stream = nuevo_coso;
	coso->size = nuevo_size;

	return valor_a_devolver;
}



t_paquete *crear_super_paquete(op_code code_op)
{
	t_paquete *super_paquete = malloc(sizeof(t_paquete));
	super_paquete->codigo_operacion = code_op;
	crear_buffer(super_paquete);
	return super_paquete;
}



void cargar_string_al_super_paquete(t_paquete *paquete, char *string)
{
	int size_string = strlen(string) + 1;

	if (paquete->buffer->size == 0)
	{
		paquete->buffer->stream = malloc(sizeof(int) + sizeof(char) * size_string);
		memcpy(paquete->buffer->stream, &size_string, sizeof(int));
		memcpy(paquete->buffer->stream + sizeof(int), string, sizeof(char) * size_string);
	}
	else
	{
		paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int) + sizeof(char) * size_string);
		memcpy(paquete->buffer->stream + paquete->buffer->size, &size_string, sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), string, sizeof(char) * size_string);
	}
	paquete->buffer->size += sizeof(int);
	paquete->buffer->size += sizeof(char) * size_string;
}

void cargar_uint32_t_al_super_paquete(t_paquete *paquete, uint32_t numero)
{
	if (paquete->buffer->size == 0)
	{
		paquete->buffer->stream = malloc(sizeof(uint32_t));
		memcpy(paquete->buffer->stream, &numero, sizeof(uint32_t));
	}
	else
	{
		paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t));
		memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(uint32_t));
	}

	paquete->buffer->size += sizeof(int);
	
}

void cargar_int_al_super_paquete(t_paquete *paquete, int numero)
{
	if (paquete->buffer->size == 0)
	{
		paquete->buffer->stream = malloc(sizeof(int));
		memcpy(paquete->buffer->stream, &numero, sizeof(int));
	}
	else
	{
		paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size, &numero, sizeof(int));
	}

	paquete->buffer->size += sizeof(int);
	
}

void liberar_array_strings(char **array) 
{ 
    if (array == NULL) return; // Si ya es null, entonces termino la función
    
    for (int i = 0; array[i] != NULL; i++) 
    {   // Sino voy recorriendo y liberando
        free(array[i]);
    }
    free(array);
}

t_buffer *recibiendo_super_paquete(int conexion)
{
	t_buffer *unBuffer = malloc(sizeof(t_buffer));
	int size;
	unBuffer->stream = recibir_buffer(&size, conexion);
	unBuffer->size = size;
	return unBuffer;
}

void *recibir_cosas_del_buffer(t_buffer *coso)
{
	if (coso->size == 0)
	{
		printf("\n[ERROR] Al intentar extraer un contenido de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if (coso->size < 0)
	{
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int size_cosas;
	void *pipon;
	memcpy(&size_cosas, coso->stream, sizeof(int));
	pipon = malloc(size_cosas);
	memcpy(pipon, coso->stream + sizeof(int), size_cosas);

	int nuevo_size = coso->size - sizeof(int) - size_cosas;
	if (nuevo_size == 0)
	{
		free(coso->stream);
		coso->stream = NULL;
		coso->size = 0;
		return pipon;
	}
	if (nuevo_size < 0)
	{
		printf("\n[ERROR_CHICLO]: BUFFER CON TAMAÑO NEGATIVO\n\n");
		// free(choclo);
		// return "";
		exit(EXIT_FAILURE);
	}
	void *nuevo_choclo = malloc(nuevo_size);
	memcpy(nuevo_choclo, coso->stream + sizeof(int) + size_cosas, nuevo_size);
	free(coso->stream);
	coso->stream = nuevo_choclo;
	coso->size = nuevo_size;

	return pipon;
}

void cargar_cosas_al_super_paquete(t_paquete *paquete, void *choclo, int size)
{
	if (paquete->buffer->size == 0)
	{
		paquete->buffer->stream = malloc(sizeof(int) + size);
		memcpy(paquete->buffer->stream, &size, sizeof(int));
		memcpy(paquete->buffer->stream + sizeof(int), choclo, size);
	}
	else
	{
		paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int) + size);
		memcpy(paquete->buffer->stream + paquete->buffer->size, &size, sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), choclo, size);
	}

	paquete->buffer->size += sizeof(int);
	paquete->buffer->size += size;
}