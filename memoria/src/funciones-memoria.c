#include "funciones-memoria.h"

//********** LEE EL ARCHIVO Y CARGA LAS INSTRUCCIONES EN UNA LISTA (LAS INSTRUCCIONES LAS DEJA DEL TIPO "t_instruccion_codigo")
t_list* leer_archivo_y_cargar_instrucciones(char* archivo_pseudocodigo)
{
    // Abro el archivo en modo lectura

    // POR AHORA DEJO LA LINEA DE ABAJO

    char* filepath = string_new(); // Creo un string para almacenar la ruta del archivo

    //string_append(&filepath, "/home/utnso/tp-2025-1c-Syscalls-Society/memoria/pseudocodigo.txt");

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

    char* linea_instruccion = malloc(256);

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



// Proceso* buscar_contexto_por_pid(int pid)
// {
//     Proceso* contexto_actual;
//     // Recorrer la lista de contextos
//     for (int i = 0; i < list_size(lista_contextos); i++) 
//     {
//         // Obtener el contexto actual
//         contexto_actual = list_get(lista_contextos, i);
        
//         // Verificar si el PID coincide
//         if(contexto_actual->pid == pid) 
//         {
//             return contexto_actual; // Retorna el contexto si lo encuentra
//         }
//     }

//     log_error(logger_memoria, "No se encontró el contexto para el PID %d", pid);
//     //exit(1); // Si no se encuentra, devolver NULL
//     return contexto_actual;
// }
Proceso* buscar_contexto_por_pid(int pid)
{    
    // Buscar en el diccionario de procesos
    Proceso* proceso = leerDeDiccionario(diccionarioProcesos, pasarUnsignedAChar(pid));
    
    if (proceso) {
        return proceso;
    }
    
    log_error(logger_memoria, "No se encontró el contexto para el PID %d", pid);
    return NULL;
}

void buscar_y_mandar_instruccion(t_buffer *buffer, int socket_cpu)
{
    
    uint32_t pid = recibir_uint32_t_del_buffer(buffer);
    uint32_t pc = recibir_uint32_t_del_buffer(buffer);
    


    Proceso* nuevo_contexto = leerDeDiccionario(diccionarioProcesos,pasarUnsignedAChar(pid)); 
    char* instruccion = obtener_instruccion_por_indice(nuevo_contexto->lista_instrucciones, pc);
    
    if(instruccion == NULL) 
        return;
    
    
    t_paquete* paquete_contexto = crear_super_paquete(CPU_RECIBE_INSTRUCCION_MEMORIA);
    cargar_uint32_t_al_super_paquete(paquete_contexto, nuevo_contexto->pid);
    cargar_string_al_super_paquete(paquete_contexto, instruccion);
    
    enviar_paquete(paquete_contexto, socket_cpu);
    
    printf("Indice: %d -- INSTRUCCION: %s \n", pc, instruccion);
    
    free(paquete_contexto);
        
    // Liberar memoria usada por el string_split
     
}

char* obtener_instruccion_por_indice(t_list* instrucciones, uint32_t indice_instruccion)
{
 	char* instruccion_actual;
 	if(indice_instruccion >= 0 && indice_instruccion < list_size(instrucciones))
    {
 		instruccion_actual = list_get(instrucciones, indice_instruccion);
 		return instruccion_actual;
 	}
 	else
    {
 		log_error(logger_memoria, "Numero de Instruccion <%d> NO VALIDA", indice_instruccion);
 		return NULL;
 	}
}
