#include "instruccion.h"

void instruccion_noop(char** parte)
{
    if(parte[1] == NULL)
    {
    usleep(10000);
    }
    else
    {
        perror("Error al ejecutar NOOP");
    }
    contexto->registros.PC++;

    check_interrupt();
}

void instruccion_escribir_memoria(char** parte)
{  
    // WRITE_MEM (Registro Dirección, Registro Datos): 
    // Lee el valor del Registro Datos y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección
    // Le pasamos a memoria el contenido a escribir (que se encuentra en registro datos) y la direccion fisica (que se encuentra en registro direccion) 

    // Leo el tamaño que  tama
    char* datos_a_escribir = parte[2];         // parte[2] = Cadena sin espacios

    // Traducimos la direccion del registro direccion
    int direccion_logica = atoi(parte[1]); // parte[1] = "0"

    //ACA HAY ALGO RARO, YA QUE NO DEBERIA COMPARAR CON AX BX CX PORQUE LLEGAN NUMEROS!! NO NOMBRES DE REGISTROS

    int direccion_fisica = mmu_traducir_direccion_logica(direccion_logica);

    if (direccion_fisica < 0) {
        log_error(logger_cpu, "Segmentation Fault al traducir la dirección lógica %d", direccion_logica);
        enviar_interrupcion_a_kernel_y_memoria(parte, SEGMENTATION_FAULT);
        return;
    }

    peticion_escritura_a_memoria(direccion_fisica, datos_a_escribir);

    // Aumento el PC para que lea la proxima instruccion
    contexto->registros.PC++;

    liberar_array_strings(parte);

    check_interrupt();

    return;
}


void instruccion_leer_memoria(char** parte)
{  
    // READ(Registro Direccion, Tamaño): 
    // Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el Registro Datos

     // Leo el tamaño que va a leer en memoria
     int tamanio = atoi(parte[2]); // parte[2] = "20"

    // Obtengo la direccion logica (registro direccion)
    int direccion_logica = atoi(parte[1]); // parte[1] = "0"

    int direccion_fisica = mmu_traducir_direccion_logica(direccion_logica);

    if (direccion_fisica < 0) {
        log_error(logger_cpu, "Segmentation Fault al traducir la dirección lógica %d", direccion_logica);
        enviar_interrupcion_a_kernel_y_memoria(parte, SEGMENTATION_FAULT);
        return;
    }

    peticion_lectura_a_memoria(direccion_fisica, tamanio);
    
    contexto->registros.PC++;

    liberar_array_strings(parte);

    check_interrupt();
}

void instruccion_goto(char** parte)
{
    if(parte[1] == NULL) {
        log_error(logger_cpu, "Falta parametro para GOTO.");
        return;
    }

    int nuevo_pc = atoi(parte[1]);
    contexto->registros.PC = nuevo_pc;
}

void peticion_escritura_a_memoria(int direccion_fisica, char* valor_registro_dato)
{
    t_paquete* paquete = crear_super_paquete(CPU_PIDE_ESCRIBIR_MEMORIA);
    cargar_int_al_super_paquete(paquete, contexto->pid);
    cargar_int_al_super_paquete(paquete, direccion_fisica);
    cargar_string_al_super_paquete(paquete, valor_registro_dato);
    enviar_paquete(paquete, socket_cpu_memoria);
    free(paquete);
}

int mmu_traducir_direccion_logica(int direccion_logica)
{
    /*int nro_pagina = direccion_logica / tamanio_pagina;
    int desplazamiento = direccion_logica % tamanio_pagina;
    int tabla_actual = contexto->pid;
    int entrada_nivel_X;

    for(int nivel = 1; nivel < cant_niveles; nivel++) {

        int potencia = pow(cant_entradas_tabla, cant_niveles - nivel);
        entrada_nivel_X = (nro_pagina / potencia) % cant_entradas_tabla;

        //si no es el ultimo consulto a memoria por la siguiente tabla
        if (nivel < cant_niveles - 1) {
            int siguiente_tabla = SOLICITO_TABLA_A_MEMORIA ////// Función que trae tabla de memoria
            tabla_actual = siguiente_tabla;

        } else {
        }
    }       */
    return 0;
}

void peticion_lectura_a_memoria(int direccion_fisica, int tamanio)
{
    t_paquete* paquete = crear_super_paquete(CPU_PIDE_LEER_MEMORIA);
    cargar_int_al_super_paquete(paquete, contexto->pid);
    cargar_int_al_super_paquete(paquete, direccion_fisica);
    cargar_int_al_super_paquete(paquete, tamanio);
    enviar_paquete(paquete, socket_cpu_memoria);
    free(paquete);
}

void solicitar_contexto_a_memoria(t_contexto_cpu* contexto)
{
    // Creo el paquete
    t_paquete* paquete = crear_super_paquete(CPU_PIDE_CONTEXTO);
    cargar_int_al_super_paquete(paquete, contexto->pid);
    cargar_int_al_super_paquete(paquete, contexto->registros.PC);
    // Envio el paquete a memoria
    enviar_paquete(paquete, socket_cpu_memoria);  //PRIMERA SOLICITUD A MEMORIA, ESPERO EL CONTEXTO TODOS LOS REGISTROS EN 0

    free(paquete);
}



// --------------- ESTO ES FETCH --------------- //// EJEMPLO CPU_PIDE_INSTRUCCION_A_MEMORIA

void fetch(int socket_cpu_memoria) 
{   
    // Obtener instrucción
    // Deberemos devolverle la instrucción correspondiente pid y al Program Counter recibido. 

    t_paquete* paquete;

    // CASO EN EL QUE SE EJECUTA  SOLAMENTE UNA INSTRUCCION COMUN Y SE VUELVE A SOLICITAR OTRA
    paquete = crear_super_paquete(CPU_PIDE_INSTRUCCION_A_MEMORIA);
    
    cargar_int_al_super_paquete(paquete, contexto->pid);

    cargar_int_al_super_paquete(paquete, contexto->registros.PC);

    // Envio el paquete a memoria
    enviar_paquete(paquete, socket_cpu_memoria);
    free(paquete);
}

void decode()
{     
    // INIT_PROC proceso1 256

    printf("Antes de el semaforo hay instruccion\n");
    sem_wait(&sem_hay_instruccion); 
    printf("Despues de el semaforo hay instruccion\n");
    while(1)
    {
        char** parte = string_split(instruccion_recibida, " "); // Divido la instrucción (que es un string) en partes  (decode)

        int instruccion_enum = (int)(intptr_t)dictionary_get(instrucciones, parte[0]); // Aca se obtiene la instrucción (el enum) a partir del diccionario

        switch (instruccion_enum) // Según la instrucción que sea, ejecuta la instruccion (execute)
        {
            case I_READ_MEM:
                //instruccion_leer_memoria(parte);  
                break;
            case I_WRITE_MEM:
                instruccion_escribir_memoria(parte); 
                break;
            case I_NOOP:
                instruccion_noop(parte);
                break;
            case I_GOTO:
                instruccion_goto(parte);
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

        if(motivo == INTERRUPCION_PID) // le envio el contexto a memoria 
        {
            printf("antes de enviar motivo de interrupcion\n");
            enviar_interrupcion_a_kernel_y_memoria(parte, motivo_interrupcion);
            printf("despues de enviar motivo de interrupcion\n");
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
        printf("--------------No hay interrupcion \n");
        
        //pthread_mutex_lock(&mutex_motivo_interrupcion);
        if(motivo_interrupcion != INTERRUPCION_PID)
        {
            motivo_interrupcion = -1;
        }
        printf("despues de motivo de interrupcion = -1 \n");
        //pthread_mutex_unlock(&mutex_motivo_interrupcion);

        fetch(socket_cpu_memoria); // Aca vuelvo a pedirle una instruccion a memoria
        
        // sem_post(&sem_nueva_instruccion);
    }
}


void cargar_registros_a_paquete(t_paquete* paquete_memoria)
{
    cargar_int_al_super_paquete(paquete_memoria, contexto->registros.AX);
    cargar_int_al_super_paquete(paquete_memoria, contexto->registros.BX);
    cargar_int_al_super_paquete(paquete_memoria, contexto->registros.CX);
    cargar_int_al_super_paquete(paquete_memoria, contexto->registros.DX);
    cargar_int_al_super_paquete(paquete_memoria, contexto->registros.EX);
    cargar_int_al_super_paquete(paquete_memoria, contexto->registros.FX);
    cargar_int_al_super_paquete(paquete_memoria, contexto->registros.GX);
    cargar_int_al_super_paquete(paquete_memoria, contexto->registros.HX);
    cargar_int_al_super_paquete(paquete_memoria, contexto->registros.PC);
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
}

// **********************************  

void iniciar_diccionario_instrucciones()
{
	instrucciones = dictionary_create(); // Creo el diccionario

    // dictionary_put(diccionario al que se agrega el par clave-valor (como dijo tomy), clave para acceder al valor en el diccionario, valor que se va a asociar con la clave en el diccionario)
	dictionary_put(instrucciones, "READ", (void*)(intptr_t)I_READ_MEM);
	dictionary_put(instrucciones, "WRITE", (void*)(intptr_t)I_WRITE_MEM);
    dictionary_put(instrucciones, "DUMP_MEMORY", (void*)(intptr_t)I_DUMP_MEMORY);
    dictionary_put(instrucciones, "IO", (void*)(intptr_t)I_IO);
    dictionary_put(instrucciones, "GOTO", (void*)(intptr_t)I_GOTO);
    dictionary_put(instrucciones, "NOOP", (void*)(intptr_t)I_NOOP);
}


void destruir_diccionarios() 
{
	dictionary_destroy(instrucciones);
	dictionary_destroy(registros);
}


void iniciar_diccionario_registros()  
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
    return -1;
}

void enviar_interrupcion_a_kernel_y_memoria(char** instruccion, op_code motivo_de_interrupcion)
{
    t_paquete *paquete_kernel_dispatch;
    t_paquete *paquete_memoria;
    
    // KERNEL -> PID, TID y motivo_de_interrupcion -> le mandamos esto a kernel para que pueda resolver la interrupcion (syscalls)
    paquete_kernel_dispatch = crear_super_paquete(motivo_de_interrupcion);
    cargar_int_al_super_paquete(paquete_kernel_dispatch, contexto->pid);

    switch (motivo_de_interrupcion)
    {    
        case SEGMENTATION_FAULT:
        //MEMORIA
            contexto->registros.PC++;
            paquete_memoria = crear_super_paquete(ENVIAR_A_MEMORIA_UN_AVISO_DE_SYSCALL);
            cargar_int_al_super_paquete(paquete_kernel_dispatch,contexto->pid);
            cargar_registros_a_paquete(paquete_memoria);
        break;
        case IO:
            // KERNEL
            cargar_int_al_super_paquete(paquete_kernel_dispatch, (int)atoi(instruccion[1]));
            // MEMORIA
            contexto->registros.PC++;
            paquete_memoria = crear_super_paquete(ENVIAR_A_MEMORIA_UN_AVISO_DE_SYSCALL);
            cargar_int_al_super_paquete(paquete_kernel_dispatch,contexto->pid);
            cargar_registros_a_paquete(paquete_memoria);
            break;
        case DUMP_MEMORY:
            // MEMORIA
            contexto->registros.PC++;
            paquete_memoria = crear_super_paquete(ENVIAR_A_MEMORIA_UN_AVISO_DE_SYSCALL);
            cargar_int_al_super_paquete(paquete_kernel_dispatch,contexto->pid);
            cargar_registros_a_paquete(paquete_memoria);
            break;
        case INIT_PROCCESS:
            // MEMORIA
            contexto->registros.PC++;
            paquete_memoria = crear_super_paquete(ENVIAR_A_MEMORIA_UN_AVISO_DE_SYSCALL);
            cargar_int_al_super_paquete(paquete_kernel_dispatch,contexto->pid);
            cargar_string_al_super_paquete(paquete_memoria, instruccion[1]); // NOMBRE DEL ARCHIVO DE PSEUDOCODIGO
            cargar_int_al_super_paquete(paquete_memoria, (int)atoi(instruccion[2]) ); // TAMANIO DEL PROCESO 
            cargar_registros_a_paquete(paquete_memoria);
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
    free(paquete_memoria);
     
    // KERNEL
    //sem_wait(&sem_ok_actualizar_contexto);
    printf("Mando paquete a kenrel para comprobar algo xd --------------------\n");
    enviar_paquete(paquete_kernel_dispatch, socket_cpu_kernel_dispatch);
    free(paquete_kernel_dispatch);
    //sem_post(&sem_pid); 
}
