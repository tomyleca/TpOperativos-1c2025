#include "instruccion.h"

void instruccion_escribir_memoria(char** parte)
{  
    // WRITE_MEM (Registro Dirección, Registro Datos): 
    // Lee el valor del Registro Datos y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección
    // Le pasamos a memoria el contenido a escribir (que se encuentra en registro datos) y la direccion fisica (que se encuentra en registro direccion) 
    // Leo el registro de datos
    char *registro_dato = parte[2];

    // Traducimos la direccion del registro direccion
    char *registro_direccion = parte[1]; // Direccion logica

    int direccion_logica = obtener_valor_registro_segun_nombre(registro_direccion);

    // Aumento el PC para que lea la proxima instruccion
    contexto->registros.PC++;

    liberar_array_strings(parte);

    check_interrupt();

    return;
}

void instruccion_leer_memoria(char** parte)
{  
    // READ_MEM(Registro Datos, Registro Dirección): 
    // Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el Registro Datos

    char *registro_datos = parte[1];
    char *registro_direccion = parte[2]; 
    
    int *valor_registro_datos = dictionary_get(registros, parte[1]); //USAMOS INT POR EL MOMENTO!!
    int *valor_registro_direccion = dictionary_get(registros, parte[2]);

    peticion_lectura_a_memoria(direccion_fisica); //...................................TODAVIA NO LA HICE.............................................................
    
    contexto->registros.PC++;

    liberar_array_strings(parte);

    check_interrupt();
}

void peticion_escritura_a_memoria(int direccion_fisica, uint32_t valor_registro_dato)
{
    t_paquete* paquete = crear_super_paquete(CPU_PIDE_ESCRIBIR_MEMORIA);
    cargar_int_al_super_paquete(paquete, contexto->pid);
    cargar_int_al_super_paquete(paquete, contexto->tid);
    cargar_int_al_super_paquete(paquete, direccion_fisica);
    cargar_int_al_super_paquete(paquete, valor_registro_dato);
    enviar_paquete(paquete, conexion_cpu_memoria);
    destruir_paquete(paquete);
}

void peticion_lectura_a_memoria(int direccion_fisica)
{
    t_paquete* paquete = crear_super_paquete(CPU_PIDE_LEER_MEMORIA);
    cargar_int_al_super_paquete(paquete, contexto->pid);
    cargar_int_al_super_paquete(paquete, direccion_fisica);
    enviar_paquete(paquete, conexion_cpu_memoria);
    destruir_paquete(paquete);
}

void enviar_interrupcion_a_kernel_y_memoria(char** instruccion, op_code motivo_de_interrupcion)
{
    t_paquete *paquete_kernel_dispatch;
    t_paquete *paquete_memoria;
    
    // MEMORIA -> le mandamos esto para que pueda actualizar el contexto
    cargar_int_al_super_paquete(paquete_memoria->buffer, contexto->pid);
    
    // KERNEL -> PID,  motivo_de_interrupcion
    paquete_kernel_dispatch = crear_super_paquete(motivo_de_interrupcion);
    cargar_int_al_super_paquete(paquete_kernel_dispatch->buffer, contexto->pid);

    switch (motivo_de_interrupcion)
    {
        case IO:
            // KERNEL
            cargar_int_al_super_paquete(paquete_kernel_dispatch->buffer, (int)atoi(instruccion[1]));
            // MEMORIA
            contexto->registros.PC++;
            paquete_memoria = crear_super_paquete(ENVIAR_A_MEMORIA_UN_AVISO_DE_SYSCALL);
            cargar_registros_a_paquete(paquete_memoria->buffer);
            break;
        case DUMP_MEMORY:
            // MEMORIA
            contexto->registros.PC++;
            paquete_memoria = crear_super_paquete(ENVIAR_A_MEMORIA_UN_AVISO_DE_SYSCALL, paquete_memoria->buffer);
            cargar_registros_a_paquete(paquete_memoria->buffer);
            break;
        default:
            log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
            break;
    }
    liberar_array_strings(instruccion); // instruccion es la variable parte
    free(instruccion_recibida);
    instruccion_recibida = NULL;
    
    // MEMORIA
    enviar_paquete(paquete_memoria, socket_cpu_memoria);
    destruir_paquete(paquete_memoria);
     
    // KERNEL
    enviar_paquete(paquete_kernel_dispatch, socket_kernel_dispatch);
    destruir_paquete(paquete_kernel_dispatch);
}


// --------------- ESTO ES FETCH --------------- //// EJEMPLO CPU_PIDE_INSTRUCCION_A_MEMORIA

void fetch(int socket_cpu_memoria) 
{   
    // Obtener instrucción
    // Deberemos devolverle la instrucción correspondiente pid y al Program Counter recibido. 

    if (contexto == NULL) 
    {
        log_error(logger_cpu, "No se pudo reservar memoria para el PCB al recibir el PCB");
        free(contexto);
    }
    
    if(valor_inicial == 1) // inicialmente tiene que estar en 0 
    {
        dictionary_destroy(registros);
    }

    iniciar_diccionario_registros();
    valor_inicial = 1;
    
    t_paquete* paquete;
    t_buffer* buffer = crear_buffer();

    // CASO EN EL QUE SE EJECUTA  SOLAMENTE UNA INSTRUCCION COMUN Y SE VUELVE A SOLICITAR OTRA
    paquete = crear_super_paquete(CPU_PIDE_INSTRUCCION_A_MEMORIA);
    
    cargar_int_al_super_paquete(buffer, contexto->pid);

    cargar_int_al_super_paquete(buffer, contexto->registros.PC);

    // Envio el paquete a memoria
    enviar_paquete(paquete, socket_cpu_memoria);
    destruir_paquete(paquete);
}

void decode()
{     
    // MOV_IN AX BX
    while(1)
    {
        char** parte = string_split(instruccion_recibida, " "); // Divido la instrucción (que es un string) en partes  (decode)

        int instruccion_enum = (int)(intptr_t)dictionary_get(instrucciones, parte[0]); // Aca se obtiene la instrucción (el enum) a partir del diccionario

        switch (instruccion_enum) // Según la instrucción que sea, ejecuta la instruccion (execute)
        {
            case I_READ_MEM:
                instruccion_leer_memoria(parte);  
                break;
            case I_WRITE_MEM:
                instruccion_escribir_memoria(parte); 
                break;
            case -1:
                log_warning(logger_cpu, "Algo paso en el interpretar instruccion!!!");
                destruir_diccionarios();
                return;
            default:
                log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
                break;

        }
        // dictionary_destroy(registros);
    }
    return;
}

void check_interrupt()
{   
    char** parte = NULL;

    //pthread_mutex_lock(&mutex_motivo_interrupcion);
    bool hay_interrupcion = flag_interrupcion;  // Leer flag
    //pthread_mutex_unlock(&mutex_motivo_interrupcion);

    if(hay_interrupcion) // hay una interrupcion
    {   
        //pthread_mutex_lock(&mutex_motivo_interrupcion);
        int motivo = motivo_interrupcion;  // Leer motivo
        //pthread_mutex_unlock(&mutex_motivo_interrupcion);

        if(motivo == INTERRUPCION_TID) // le envio el contexto a memoria 
        {
            enviar_interrupcion_a_kernel_y_memoria(parte, motivo_interrupcion);

            //pthread_mutex_lock(&mutex_motivo_interrupcion);
            flag_interrupcion = false;
            motivo_interrupcion = -1;
            //pthread_mutex_unlock(&mutex_motivo_interrupcion);
            
            //sem_post(&sem_interrupcion);
            
        } else {
            log_error(logger_cpu, "Motivo de interrupcion inesperado: %d", motivo);
        }

        // motivo de interrupcion , tid, char: instruccion 
        printf("ADENTRO del mutex check instruccion 2/n");
        
        free(instruccion_recibida);
        instruccion_recibida = NULL; 
        
    } else {

        free(instruccion_recibida);
        instruccion_recibida = NULL;
        printf("--------------No hay interrupcion por quantum \n");
        
        //pthread_mutex_lock(&mutex_motivo_interrupcion);
        if(motivo_interrupcion != INTERRUPCION_TID)
        {
            motivo_interrupcion = -1;
        }
        printf("despues de motivo de interrupcion = -1 \n");
        //pthread_mutex_unlock(&mutex_motivo_interrupcion);

        fetch(socket_cpu_memoria); // Aca vuelvo a pedirle una instruccion a memoria
        
        // sem_post(&sem_nueva_instruccion);
    }
}


void cargar_registros_a_paquete(t_buffer* buffer_memoria)
{
    cargar_int_al_super_paquete(contexto->registros.AX);
    cargar_int_al_super_paquete(contexto->registros.BX);
    cargar_int_al_super_paquete(contexto->registros.CX);
    cargar_int_al_super_paquete(contexto->registros.DX);
    cargar_int_al_super_paquete(contexto->registros.EX);
    cargar_int_al_super_paquete(contexto->registros.FX);
    cargar_int_al_super_paquete(contexto->registros.GX);
    cargar_int_al_super_paquete(contexto->registros.HX);
    cargar_int_al_super_paquete(contexto->registros.PC);
    cargar_int_al_super_paquete(contexto->registros.BASE);
    cargar_int_al_super_paquete(contexto->registros.LIMITE);
}


void cargar_registros(t_buffer* buffer)
{   
    contexto->registros.AX = recibir_int_del_buffer(buffer);
    contexto->registros.BX = recibir_int_del_buffer(buffer);
    contexto->registros.CX = recibir_int_del_buffer(buffer);
    contexto->registros.DX = recibir_int_del_buffer(buffer);
    contexto->registros.EX = recibir_int_del_buffer(buffer);
    contexto->registros.FX = recibir_int_del_buffer(buffer);
    contexto->registros.GX = recibir_int_del_buffer(buffer);
    contexto->registros.HX = recibir_int_del_buffer(buffer);
    contexto->registros.PC = recibir_int_del_buffer(buffer);
    contexto->registros.BASE = recibir_int_del_buffer(buffer);
    contexto->registros.LIMITE = recibir_int_del_buffer(buffer);
}

// **********************************  

void iniciar_diccionario_instrucciones(enum_instrucciones instrucciones)
{
	instrucciones = dictionary_create(); // Creo el diccionario

    // dictionary_put(diccionario al que se agrega el par clave-valor (como dijo tomy), clave para acceder al valor en el diccionario, valor que se va a asociar con la clave en el diccionario)
	dictionary_put(instrucciones, "READ_MEM", (void*)(intptr_t)I_READ_MEM);
	dictionary_put(instrucciones, "WRITE_MEM", (void*)(intptr_t)I_WRITE_MEM);

    dictionary_put(instrucciones, "DUMP_MEMORY", (void*)(intptr_t)I_DUMP_MEMORY);
    dictionary_put(instrucciones, "IO", (void*)(intptr_t)I_IO);
}


void destruir_diccionarios(t_registros registros, enum_instrucciones instrucciones) 
{
	dictionary_destroy(instrucciones);
	dictionary_destroy(registros);
}


void iniciar_diccionario_registros(t_registros registros)  
{
	registros = dictionary_create();
	dictionary_put(registros, "PC", &contexto->registros.PC);
	dictionary_put(registros, "AX", &contexto->registros.AX);
	dictionary_put(registros, "BX", &contexto->registros.BX);
	dictionary_put(registros, "CX", &contexto->registros.CX);
	dictionary_put(registros, "DX", &contexto->registros.DX);
	dictionary_put(registros, "EX", &contexto->registros.EX);
	dictionary_put(registros, "FX", &contexto->registros.FX);
	dictionary_put(registros, "GX", &contexto->registros.GX);
	dictionary_put(registros, "HX", &contexto->registros.HX);
}


int obtener_valor_registro_segun_nombre(char* nombre_registro)
{
    if(strcmp(nombre_registro,"AX")==0)
    {
        return contexto->registros.AX;
    } else if(strcmp(nombre_registro,"BX")==0)
    {
        return contexto->registros.BX;
    } else if(strcmp(nombre_registro,"CX")==0)
    {
        return contexto->registros.CX;
        
    } else if(strcmp(nombre_registro,"DX")==0)
    {
        return contexto->registros.DX;
    } else if(strcmp(nombre_registro,"EX")==0)
    {
        return contexto->registros.EX;
    } else if(strcmp(nombre_registro,"FX")==0)
    {
        return contexto->registros.FX;
    }
     else if(strcmp(nombre_registro,"GX")==0)
    {
        return contexto->registros.GX;
    } else if(strcmp(nombre_registro,"HX")==0)
    {
        return contexto->registros.HX;
    } else if(strcmp(nombre_registro,"PC")==0)
    {
        return contexto->registros.PC;
    } 
     else if(strcmp(nombre_registro,"BASE")==0)
    {
        return contexto->registros.BASE;
    } 
     else if(strcmp(nombre_registro,"LIMITE")==0)
    {
        return contexto->registros.LIMITE;
    } 
    return -1;
}