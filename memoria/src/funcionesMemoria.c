#include "funcionesMemoria.h"

//********** LEE EL ARCHIVO Y CARGA LAS INSTRUCCIONES EN UNA LISTA (LAS INSTRUCCIONES LAS DEJA DEL TIPO "t_instruccion_codigo")
t_list* leer_archivo_y_cargar_instrucciones(char* archivo_pseudocodigo)
{
    // Abro el archivo en modo lectura

    // EN CONFIG ME DICE DONDE ESTAN LOS PATH
    // char* direccion = strcat(config_memoria->path_instrucciones, archivo_pseudocodigo);
    // ANTES DE EJECUTAR AGREGARLE AL CONFIG DE MEMORIA->PATH_INSTRUCCIONES UNA BARRA

    // POR AHORA DEJO LA LINEA DE ABAJO
    // para no hacer que todos tengan que crear una carpeta "sripts-pruebas"

    char* filepath = string_new(); // Creo un string para almacenar la ruta del archivo

    string_append(&filepath, "../ruta del pseudocodigo xdxd");
    // string_append(&filepath, "../../kernel/pruebas/"); // PARA VALGRIND DEJAR ESTAR Y COMENTAR LA DE ARRIBA :) 

    string_append(&filepath, archivo_pseudocodigo);

    printf("Ruta del archivo: %s\n", filepath);

    FILE* archivo = fopen(filepath, "r"); // abre el archivo en modo lectura

    t_list* instrucciones = list_create(); // Creo una lista para almacenar todas las instrucciones 
    
    char* instruccion_formateada = NULL;
    int i = 0;
    if (archivo == NULL) {
        log_error(logger_memoria, "No se puede abrir el archivo_pseudocodigo para leer instrucciones.");  
        return instrucciones;
    }

    char* linea_instruccion = malloc(256 * sizeof(int));

    while (fgets(linea_instruccion, 256, archivo)) 
    {
    	int size_linea_actual = strlen(linea_instruccion);
    	if(size_linea_actual > 2)
        {
    		if(linea_instruccion[size_linea_actual - 1] == '\n')
            {
                // Aca se elimina el salto de linea \n
				char* linea_limpia = string_new();
				string_n_append(&linea_limpia, linea_instruccion, size_linea_actual - 1);
				free(linea_instruccion);
				linea_instruccion = malloc(256 * sizeof(int));
				strcpy(linea_instruccion, linea_limpia);
                free(linea_limpia);
    		}
    	}
        
    	//------------------------------------------------------------------------------------------------------------------------------------------

        char** l_instrucciones = string_split(linea_instruccion, " ");  // Separamos la instruccion y parametros por " "
        while (l_instrucciones[i]) 
        {
        	i++;
        }

        // if (!es_instruccion_valida(l_instrucciones[0])) {
        //     log_error(logger_memoria, "Instrucción no válida en pseudocódigo: %s", l_instrucciones[0]);
        //     return NULL;
        // }

        t_instruccion_codigo* pseudo_cod = malloc(sizeof(t_instruccion_codigo));
        pseudo_cod->mnemonico = strdup(l_instrucciones[0]); // Mnemonico es la instruccion 

        pseudo_cod->primero_parametro = (i > 1) ? strdup(l_instrucciones[1]) : NULL;
        pseudo_cod->segundo_parametro = (i > 2) ? strdup(l_instrucciones[2]) : NULL;
        pseudo_cod->tercero_parametro = (i > 3) ? strdup(l_instrucciones[3]) : NULL;

        if(i == 3){
            instruccion_formateada = string_from_format("%s %s %s", pseudo_cod->mnemonico, pseudo_cod->primero_parametro, pseudo_cod->segundo_parametro);

        } else if (i == 2) {
            instruccion_formateada = string_from_format("%s %s", pseudo_cod->mnemonico, pseudo_cod->primero_parametro);

        } else {
            instruccion_formateada = strdup(pseudo_cod->mnemonico);
        }

        printf("Instruccion leida y cargada %s\n", instruccion_formateada);
        list_add(instrucciones, instruccion_formateada); // Obtenemos una instruccion
        // free(instruccion_formateada);

        for (int j = 0; j < i; j++) {
            free(l_instrucciones[j]);
        }

        free(l_instrucciones);
        
        free(pseudo_cod->mnemonico);
		if(pseudo_cod->primero_parametro) free(pseudo_cod->primero_parametro);
		if(pseudo_cod->segundo_parametro) free(pseudo_cod->segundo_parametro);
        if(pseudo_cod->tercero_parametro) free(pseudo_cod->tercero_parametro);

		free(pseudo_cod);
        i = 0; // Restablece la cuenta para la próxima iteración
    }

    fclose(archivo);
    free(linea_instruccion);
    free(filepath);
    return instrucciones;
}

void crear_pid(t_contexto* nuevo_contexto, t_info_kernel* info_kernel)
{
    // Crear e inicializar el contexto del hilo (TCB)
    t_pids* nuevo_pid = malloc(sizeof(t_pids));
    if (nuevo_pid == NULL) {
        log_error(logger_memoria, "Error al asignar memoria para el PID del proceso %d", info_kernel->pid);
        free(nuevo_contexto); // Liberar memoria del contexto antes de salir
        exit(1);
    }

    nuevo_pid->pid = info_kernel->pid;
    
    // Inicializar registros del TID
    nuevo_pid->ax = 0;
    nuevo_pid->bx = 0;
    nuevo_pid->cx = 0;
    nuevo_pid->dx = 0;
    nuevo_pid->ex = 0;
    nuevo_pid->fx = 0;
    nuevo_pid->gx = 0;
    nuevo_pid->hx = 0;
    nuevo_pid->pc = 0; 

    // Leer el archivo de pseudocódigo y cargar las instrucciones
    nuevo_pid->pseudocodigo = strdup(info_kernel->archivo_pseudocodigo);
    
    nuevo_pid->instrucciones = leer_archivo_y_cargar_instrucciones(nuevo_pid->pseudocodigo);
    
    // Agregar el PID al contexto del proceso
    list_add(nuevo_contexto->lista_pids, nuevo_pid);
    
}


t_contexto* buscar_contexto_por_pid(int pid)
{
    // Recorrer la lista de contextos
    for (int i = 0; i < list_size(lista_contextos); i++) 
    {
        // Obtener el contexto actual
        t_contexto* contexto_actual = list_get(lista_contextos, i);
        
        // Verificar si el PID coincide
        if(contexto_actual->pid == pid) 
        {
            return contexto_actual; // Retorna el contexto si lo encuentra
        }
    }

    log_error(logger_memoria, "No se encontró el contexto para el PID %d", pid);
    exit(1); // Si no se encuentra, devolver NULL
    return 0;
}