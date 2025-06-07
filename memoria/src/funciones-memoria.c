#include "funciones-memoria.h"

//********** LEE EL ARCHIVO Y CARGA LAS INSTRUCCIONES EN UNA LISTA (LAS INSTRUCCIONES LAS DEJA DEL TIPO "t_instruccion_codigo")
t_list* leer_archivo_y_cargar_instrucciones(char* archivo_pseudocodigo)
{
    char* filepath = string_from_format("%s", archivo_pseudocodigo);
    FILE* archivo = fopen(filepath, "r");
    if (!archivo) {
        log_error(logger_memoria, "No se pudo abrir el archivo de pseudocódigo: %s", filepath);
        free(filepath);
        return NULL;
    }

    t_list* instrucciones = list_create();
    char* linea_instruccion = malloc(256 * sizeof(char));
    int i = 0;
    char* instruccion_formateada;

    while (fgets(linea_instruccion, 256, archivo)) {
        int size_linea_actual = strlen(linea_instruccion);
        if(size_linea_actual > 2) {
            if(linea_instruccion[size_linea_actual - 1] == '\n') {
                // Eliminar el salto de línea
                linea_instruccion[size_linea_actual - 1] = '\0';
            }
        }
        
        char** l_instrucciones = string_split(linea_instruccion, " ");
        i = 0;
        while (l_instrucciones[i]) {
            i++;
        }

        t_instruccion_codigo* pseudo_cod = malloc(sizeof(t_instruccion_codigo));
        pseudo_cod->mnemonico = strdup(l_instrucciones[0]);
        pseudo_cod->primero_parametro = (i > 1) ? strdup(l_instrucciones[1]) : NULL;
        pseudo_cod->segundo_parametro = (i > 2) ? strdup(l_instrucciones[2]) : NULL;
        pseudo_cod->tercero_parametro = (i > 3) ? strdup(l_instrucciones[3]) : NULL;

        if(i == 3) {
            instruccion_formateada = string_from_format("%s %s %s", 
                pseudo_cod->mnemonico, 
                pseudo_cod->primero_parametro, 
                pseudo_cod->segundo_parametro);
        } else if (i == 2) {
            instruccion_formateada = string_from_format("%s %s", 
                pseudo_cod->mnemonico, 
                pseudo_cod->primero_parametro);
        } else {
            instruccion_formateada = strdup(pseudo_cod->mnemonico);
        }

        printf("Instruccion leida y cargada %s\n", instruccion_formateada);
        list_add(instrucciones, instruccion_formateada);

        // Liberar memoria
        for (int j = 0; j < i; j++) {
            free(l_instrucciones[j]);
        }
        free(l_instrucciones);
        
        free(pseudo_cod->mnemonico);
        if(pseudo_cod->primero_parametro) free(pseudo_cod->primero_parametro);
        if(pseudo_cod->segundo_parametro) free(pseudo_cod->segundo_parametro);
        if(pseudo_cod->tercero_parametro) free(pseudo_cod->tercero_parametro);
        free(pseudo_cod);
    }

    fclose(archivo);
    free(linea_instruccion);
    free(filepath);
    return instrucciones;
}

void crear_pid(t_contexto* nuevo_contexto, t_info_kernel info_kernel)
{  

     // Inicializar el contexto principal del proceso
    nuevo_contexto->pid = info_kernel.pid;  
    nuevo_contexto->datos_pid.pc = 0;
    nuevo_contexto->tamanio_proceso = info_kernel.tamanio_proceso;

    printf("Ruta recibida: '%s'\n", info_kernel.archivo_pseudocodigo);

    nuevo_contexto->datos_pid.pseudocodigo = strdup(info_kernel.archivo_pseudocodigo);
    nuevo_contexto->datos_pid.instrucciones = leer_archivo_y_cargar_instrucciones(nuevo_contexto->datos_pid.pseudocodigo);
    list_add(lista_contextos, nuevo_contexto);
    // Crear la tabla de primer nivel y guardar el ID en el contexto
    //nuevo_contexto->id_tabla_primer_nivel = crear_tabla_nivel(1)
}

Proceso* buscar_contexto_por_pid(int pid)
{
    Proceso* contexto_actual = NULL; // Inicializamos a NULL
    // Recorrer la lista de contextos
    for (int i = 0; i < list_size(lista_contextos); i++) 
    {
        // Obtener el contexto actual
        contexto_actual = list_get(lista_contextos, i);
        
        // Verificar si el PID coincide
        if(contexto_actual->pid == pid) 
        {
            return contexto_actual; // Retorna el contexto si lo encuentra
        }
    }

    log_error(logger_memoria, "No se encontró el contexto para el PID %d", pid);
    return NULL; // Retornamos NULL si no se encuentra
}

void buscar_y_mandar_instruccion(t_buffer *buffer, int socket_cpu)
{
    int pid = recibir_int_del_buffer(buffer);
    uint32_t pc = recibir_uint32_t_del_buffer(buffer);
    
    Proceso* nuevo_contexto = leerDeDiccionario(diccionarioProcesos,pasarUnsignedAChar(pid)); 
    if (!nuevo_contexto) {
        log_error(logger_memoria, "No se encontró el proceso con PID %d", pid);
        return;
    }

    char* instruccion = obtener_instruccion_por_indice(nuevo_contexto->lista_instrucciones, pc++);
    if (!instruccion) {
        log_error(logger_memoria, "No se pudo obtener la instrucción para el proceso %d en PC %d", pid, pc);
        return;
    }
    
    printf("Indice: %d -- INSTRUCCION: %s \n", pc, instruccion);
    
    t_paquete* paquete_contexto = crear_super_paquete(CPU_RECIBE_INSTRUCCION_MEMORIA);
    cargar_int_al_super_paquete(paquete_contexto, nuevo_contexto->pid);
    cargar_string_al_super_paquete(paquete_contexto, instruccion);
    
    char** partes = string_split(instruccion, " ");
    
    enviar_paquete(paquete_contexto, socket_cpu);
    free(paquete_contexto);
    
    // Liberar memoria usada por el string_split
    liberar_array_strings(partes);   
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

/*void enviar_contexto(Proceso* contexto_proceso, int socket_cpu)
{   
    // Verificar que ambos contextos no sean NULL
    if (contexto_proceso == NULL || contexto_proceso->pid == NULL) 
    {
        log_error(logger_memoria, "Error al enviar contexto: Contexto o PID son NULL");
        return;
    }
    
    // Enviamos contexto de ejecucion a CPU
    t_paquete* paquete_contexto = crear_super_paquete(CPU_RECIBE_CONTEXTO);
    
    cargar_int_al_super_paquete(paquete_contexto, contexto_proceso->pid);
    cargar_string_al_super_paquete(paquete, contexto_proceso->lista_instrucciones)
   
    enviar_paquete(paquete_contexto, socket_cpu);

    free(paquete_contexto);
}*/
