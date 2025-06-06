#include "instruccion.h"

void instruccion_noop(char** parte)
{
    log_instruccion(parte);

    if(parte[1] == NULL)
    {
    usleep(10000);
    }
    else
    {
        perror("Error al ejecutar NOOP");
    }

    check_interrupt();
}

void instruccion_escribir_memoria(char** parte)
{  
    // WRITE_MEM (Registro Dirección, Registro Datos): 
    // Lee el valor del Registro Datos y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección
    // Le pasamos a memoria el contenido a escribir (que se encuentra en registro datos) y la direccion fisica (que se encuentra en registro direccion) 
    log_instruccion(parte);

    char* accion;

    if(strcmp(parte[0],"WRITE")==0)
    {
        accion = "ESCRIBIR";
    }

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

    log_info(logger_cpu, "PID: <%d> - Acción: <%s> - Dirección Física: <%u> - Valor: <%s>", contexto->pid, accion ,direccion_fisica, datos_a_escribir);

    peticion_escritura_a_memoria(direccion_fisica, datos_a_escribir);


    // Aumento el PC para que lea la proxima instruccion

    liberar_array_strings(parte);

    return;
}


void instruccion_leer_memoria(char** parte)
{  
    // READ(Registro Direccion, Tamaño): 
    // Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el Registro Datos
    log_instruccion(parte);
    char* accion; 

    if(strcmp(parte[0],"READ")==0)
    {
        accion = "LEER";
    }
    

     // Leo el tamaño que va a leer en memoria
     int tamanio = atoi(parte[2]); // parte[2] = "20"

    // Obtengo la direccion logica (registro direccion)
    int direccion_logica = atoi(parte[1]); // parte[1] = "0"

    int direccion_fisica = buscar_en_tlb(direccion_logica);

    if (direccion_fisica < 0) {
        log_error(logger_cpu, "Segmentation Fault al traducir la dirección lógica %d", direccion_logica);
        enviar_interrupcion_a_kernel_y_memoria(parte, SEGMENTATION_FAULT);
        return;
    }

    log_info(logger_cpu, "PID: <%d> - Acción: <%s> - Dirección Física: <%u> - Valor: <%d>", contexto->pid, accion ,direccion_fisica, tamanio);

    peticion_lectura_a_memoria(direccion_fisica, tamanio);
    
    liberar_array_strings(parte);
}

void instruccion_goto(char** parte)
{
    log_instruccion(parte);

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


int buscar_en_tlb(int direccion_logica) {
    int nro_pagina = direccion_logica / tamanio_pagina;
    int desplazamiento = direccion_logica % tamanio_pagina;

    // Recorrer la lista_tlb para buscar la entrada con la misma nro_pagina
    // Si pasa y devuelve algo dentro del for, es TLB HIT y no hace nada mas que esto.
    for (int i = 0; i < list_size(lista_tlb); i++) {
        EntradaTLB *entrada = list_get(lista_tlb, i);
        if (entrada->nro_pagina == nro_pagina) {
            if (strcmp(reemplazo_tlb, "FIFO") == 0) {
            log_info(logger_cpu, "PID: <%d> - TLB HIT - Pagina: <%d>", contexto->pid, nro_pagina);
            int direccion_fisica = entrada->nro_marco * tamanio_pagina + desplazamiento;
            return direccion_fisica;
            }
            else if (strcmp(reemplazo_tlb, "LRU") == 0) {
                log_info(logger_cpu, "PID: <%d> - TLB HIT - Pagina: <%d>", contexto->pid, nro_pagina);
                int direccion_fisica = entrada->nro_marco * tamanio_pagina + desplazamiento;
                entrada->timestamp = obtener_timestamp_actual(); 
                return direccion_fisica;
            }
            else{
                log_error(logger_cpu, "ERROR: No existe el algoritmo pasado por config_cpu");
            }
        }
    }
    
    // TLB miss: consultar MMU y agregar la nueva entrada a la TLB
    log_info(logger_cpu, "PID: <%d> - TLB MISS - Pagina: <%d>", contexto->pid, nro_pagina);
    int direccion_fisica = mmu_traducir_direccion_logica(direccion_logica);
    
    return direccion_fisica;
}

void agregar_a_tlb(int pid, int nro_pagina, int nro_marco) {
    // Crear una nueva entrada para la TLB
    TLB_proceso = malloc(sizeof(EntradaTLB));
    TLB_proceso->pid = pid;
    TLB_proceso->nro_pagina = nro_pagina;
    TLB_proceso->nro_marco = nro_marco;
    TLB_proceso->timestamp = obtener_timestamp_actual(); 
    
    if (list_size(lista_tlb) >= entradas_tlb) {
        if (strcmp(reemplazo_tlb, "FIFO") == 0) {
            EntradaTLB* entrada_removida = list_remove(lista_tlb, 0);
            free(entrada_removida);
        } 
        else if(strcmp(reemplazo_tlb, "LRU") == 0) {
            int indice_mas_viejo = 0;
            for (int i = 1; i < list_size(lista_tlb); i++) {
                EntradaTLB *entrada = list_get(lista_tlb, i);
                if (entrada->timestamp < ((EntradaTLB *)list_get(lista_tlb, indice_mas_viejo))->timestamp) {
                    indice_mas_viejo = i;
                }
            }
            EntradaTLB* entrada_removida = list_remove(lista_tlb, indice_mas_viejo);
            free(entrada_removida);
        }
    }
    // Agregar la nueva entrada a la TLB
    list_add(lista_tlb, TLB_proceso);
}

int obtener_timestamp_actual() {
    return timestamp_actual++;// ACTUALIZO UN TIMESTAMP GLOBAL, para que cambie y siempre sea uno mayor que otro. 
}


int mmu_traducir_direccion_logica(int direccion_logica) {

    int nro_pagina = direccion_logica / tamanio_pagina;
    int desplazamiento = direccion_logica % tamanio_pagina;
    int tabla_actual = contexto->pid;  // Empezamos con la tabla de primer nivel asociada al proceso
    int entrada_nivel_X;
    int nro_marco = -1;

    for (int nivel = 1; nivel <= cant_niveles; nivel++) {
        int potencia = (int) pow(cant_entradas_tabla, cant_niveles - nivel);
        entrada_nivel_X = (nro_pagina / potencia) % cant_entradas_tabla;

        if (nivel < cant_niveles) {
            //TODO HACER TODO ESTO
            //tabla_actual = solicitar_tabla_a_memoria(tabla_actual, entrada_nivel_X);
        } else {
           // nro_marco = solicitar_marco_a_memoria(tabla_actual, entrada_nivel_X);
        }
    }

    if (nro_marco == -1) {
        // Si no estaba en memoria, podriamos necesitar pedir a swap
        //nro_marco = solicitar_pagina_a_swap(tabla_actual, entrada_nivel_X);//TODO hacer esta funcion
        //agregar_a_tlb(contexto->pid, nro_pagina, nro_marco); // TODO esto a verificar dsp
    }

    // Calculamos dirección física
    int direccion_fisica = nro_marco * tamanio_pagina + desplazamiento;
    agregar_a_tlb(contexto->pid, nro_pagina, nro_marco);
    return direccion_fisica;
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

    log_info(logger_cpu, "## PID: %d - FETCH - Program Counter: %d", contexto->pid, contexto->registros.PC);

    // Envio el paquete a memoria
    enviar_paquete(paquete, socket_cpu_memoria);
    free(paquete);
}

void decode()
{     
        printf("Antes de el semaforo hay instruccion\n");
        sem_wait(&sem_hay_instruccion); 
        printf("Despues de el semaforo hay instruccion\n");
        
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
            case I_NOOP:
                instruccion_noop(parte);
                break;
            case I_GOTO:
                instruccion_goto(parte);
                break;
            case I_IO:
                syscall_IO(parte);
                break;
            case I_DUMP_MEMORY:
                syscallDUMP_MEMORY(parte);
                break;
            case I_INIT_PROCESS:
                syscallINIT_PROC(parte);
                break;
            case I_EXIT:
                syscallEXIT(parte);
                break;
            case -1:
                log_warning(logger_cpu, "Algo paso en el interpretar instruccion!!!");
                destruir_diccionarios();
                    return;
            default:
                log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
                break;
            }
}
     


void check_interrupt()
{   
    char** parte = NULL;
    pthread_mutex_lock(&mutex_motivo_interrupcion);
    bool hay_interrupcion = flag_interrupcion;  // Leer flag
    pthread_mutex_unlock(&mutex_motivo_interrupcion);

    if(hay_interrupcion) // hay una interrupcion
    {   
        pthread_mutex_lock(&mutex_motivo_interrupcion);
        int motivo = motivo_interrupcion;  // Leer motivo
        pthread_mutex_unlock(&mutex_motivo_interrupcion);

        if(motivo == INTERRUPCION_PID) // le envio el contexto a memoria 
        {
            printf("antes de enviar motivo de interrupcion\n");
            enviar_interrupcion_a_kernel_y_memoria(parte, motivo_interrupcion);
            printf("despues de enviar motivo de interrupcion\n");
            pthread_mutex_lock(&mutex_motivo_interrupcion);
            flag_interrupcion = false;
            motivo_interrupcion = -1;
            pthread_mutex_unlock(&mutex_motivo_interrupcion);
            
            sem_post(&sem_interrupcion);
            
        } else {
            log_error(logger_cpu, "Motivo de interrupcion inesperado: %d", motivo);
        }

        printf("ADENTRO del mutex check instruccion 2/n");
        free(instruccion_recibida);
        instruccion_recibida = NULL; 

    } else {

        free(instruccion_recibida);
        instruccion_recibida = NULL;
        printf("--------------No hay interrupcion \n");
        
        pthread_mutex_lock(&mutex_motivo_interrupcion);
        if(motivo_interrupcion != INTERRUPCION_PID)
        {
            motivo_interrupcion = -1;
        }
        printf("despues de motivo de interrupcion = -1 \n");
        pthread_mutex_unlock(&mutex_motivo_interrupcion);
        contexto->registros.PC++;
        ciclo_instruccion(socket_cpu_memoria); // Aca vuelvo a pedirle una instruccion a memoria
    }
}

void ciclo_instruccion(int socket_cpu_memoria)
{
    fetch(socket_cpu_memoria);

    decode();

    check_interrupt(); 

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
    dictionary_put(instrucciones, "INIT_PROC", (void*) (intptr_t)I_INIT_PROCESS);
    dictionary_put(instrucciones, "EXIT", (void*) (intptr_t)I_EXIT);
}


void destruir_diccionarios() 
{
	dictionary_destroy(instrucciones);
}

void enviar_interrupcion_a_kernel_y_memoria(char** instruccion, op_code motivo_de_interrupcion)
{
    t_paquete *paquete_kernel_dispatch;
    t_paquete *paquete_memoria;
    
    
    paquete_kernel_dispatch = crear_super_paquete(motivo_de_interrupcion);
    cargar_int_al_super_paquete(paquete_kernel_dispatch, contexto->pid);

    switch (motivo_de_interrupcion)
    {    
        case SEGMENTATION_FAULT: //TODO esto va en otro lado
        //MEMORIA
            contexto->registros.PC++;
            paquete_kernel_dispatch->codigo_operacion=SEGMENTATION_FAULT;
            paquete_memoria = crear_super_paquete(SEGMENTATION_FAULT);
            cargar_int_al_super_paquete(paquete_kernel_dispatch,contexto->pid);
            cargar_int_al_super_paquete(paquete_kernel_dispatch,contexto->registros.PC);
            
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
    printf("Mando paquete a kenrel para comprobar algo xd --------------------\n");
    enviar_paquete(paquete_kernel_dispatch, socket_cpu_kernel_dispatch);
    free(paquete_kernel_dispatch);
    //sem_post(&sem_pid); 
}

void log_instruccion(char** parte) {
    if (parte == NULL || parte[0] == NULL) {
        log_info(logger_cpu, "## PID: %d - Ejecutando: <instrucción vacía o nula>", contexto->pid);
        return;
    }

    if (parte[1] && parte[2]) {
        log_info(logger_cpu, "## PID: %d - Ejecutando: %s - %s %s", contexto->pid, parte[0], parte[1], parte[2]);
    } else if (parte[1]) {
        log_info(logger_cpu, "## PID: %d - Ejecutando: %s - %s", contexto->pid, parte[0], parte[1]);
    } else {
        log_info(logger_cpu, "## PID: %d - Ejecutando: %s -", contexto->pid, parte[0]);
    }
}



// *********CACHE**********

typedef struct 
{
    int pid;                    
    int nro_pagina;            
    int nro_marco;             
    char* contenido;           
    bool bit_referencia;       
    bool bit_modificacion;  //para el CLOCK modificado   
    bool bit_validez;           
} EntradaCache;

EntradaCache* cache_paginas;
int puntero_clock;  

void inicializar_cache() 
{
    if (entradas_cache <= 0) {
        cache_paginas = NULL;
        return;
    }

    cache_paginas = malloc(sizeof(EntradaCache) * entradas_cache);
    puntero_clock = 0;
    
    // Inicializo todas las entradas  deshabilitadas
    for (int i = 0; i < entradas_cache; i++) {
        cache_paginas[i].pid = -1;
        cache_paginas[i].nro_pagina = -1;
        cache_paginas[i].nro_marco = -1;
        cache_paginas[i].contenido = malloc(tamanio_pagina);
        memset(cache_paginas[i].contenido, 0, tamanio_pagina);
        cache_paginas[i].bit_referencia = false;
        cache_paginas[i].bit_modificacion = false;
        cache_paginas[i].bit_validez = false;
    }
}

int buscar_en_cache(int pid, int nro_pagina) 
{   
    for (int i = 0; i < entradas_cache; i++) {
        if (cache_paginas[i].bit_validez && 
            cache_paginas[i].pid == pid && 
            cache_paginas[i].nro_pagina == nro_pagina) {
            
            cache_paginas[i].bit_referencia = true;
            
            log_info(logger_cpu, "PID: <%d> - CACHE HIT - Página: <%d>", pid, nro_pagina);
            return i; // Devuelve indice
        }
    }
    
    log_info(logger_cpu, "PID: <%d> - CACHE MISS - Página: <%d>", pid, nro_pagina);
    return -1; // No encontrada
}

char leer_byte_con_cache(int direccion_logica) 
{
    // Si caché está deshabilitada, entonces voy directo a memoria
    if (cache_paginas == NULL) {
        int direccion_fisica = buscar_en_tlb(direccion_logica);
        if (direccion_fisica == -1) {
            return 0;
        }
    
        


    }
    
    //CACHÉ HABILITADA

    //Descompongo direccion logica
    int nro_pagina = direccion_logica / tamanio_pagina;
    int desplazamiento = direccion_logica % tamanio_pagina;
    
    // Busco en caché 
    int indice_cache = buscar_en_cache(contexto->pid, nro_pagina);
    if (indice_cache != -1) {
        // CACHE HIT -> pagina encontrada
        return cache_paginas[indice_cache].contenido[desplazamiento];
    }
    
    // CACHE MISS -> busco pagina en memoria fisica
    int direccion_fisica = buscar_en_tlb(direccion_logica);
    if (direccion_fisica == -1) {
        return 0;
    }
    
    int nro_marco = direccion_fisica / tamanio_pagina;
    //cargar_pagina_en_cache(); FALTA ESTA FUNCION!!!
    
    // Busco de nuevo en caché
    indice_cache = buscar_en_cache(contexto->pid, nro_pagina);
    if (indice_cache != -1) {
        return cache_paginas[indice_cache].contenido[desplazamiento];
    }
    
    return 0; 
}

int algoritmo_clock() 
{
    int inicio = puntero_clock;
    
    for (int i = 0; i < entradas_cache; i++) {
        if (cache_paginas[puntero_clock].bit_referencia == false) {
            int victima = puntero_clock;
            puntero_clock = (puntero_clock + 1) % entradas_cache; //avanzo
            return victima;
        }

        cache_paginas[puntero_clock].bit_referencia = false; //si es true le da otra vuelta
        puntero_clock = (puntero_clock + 1) % entradas_cache; //avanzo
    }
    
    //si todas estaban en true, ahora todas tienen false y no devuelve victima
    int victima = puntero_clock;
    puntero_clock = (puntero_clock + 1) % entradas_cache;

    return victima;
}



