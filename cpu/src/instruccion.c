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

    string_array_destroy(parte);

}

void instruccion_escribir_memoria(char** parte)
{  
    // WRITE_MEM (Registro Dirección, Registro Datos): 
    // Lee el valor del Registro Datos y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección
    // Le pasamos a memoria el contenido a escribir (que se encuentra en registro datos) y la direccion fisica (que se encuentra en registro direccion) 
    log_instruccion(parte);


    // Leo el tamaño que  tama
    char* datos_a_escribir = parte[2];         // parte[2] = Cadena sin espacios

    // Traducimos la direccion del registro direccion
    int direccion_logica = atoi(parte[1]); // parte[1] = "0"

    if (cache_paginas != NULL && entradas_cache > 0) //Si la cache esta habilitada trabajo directamente en cache
    {
        escribir_cache(direccion_logica,datos_a_escribir);
    }
    else //sino le pido la direccion lógica a memoria y escribo en memoria
    {    int direccion_fisica = traducir_direccion_logica(direccion_logica);

        if (direccion_fisica < 0) {
            log_error(logger_cpu, "Segmentation Fault al traducir la dirección lógica %d", direccion_logica);
            exit(1);
            return;
        }

        peticion_escritura_a_memoria(direccion_fisica, datos_a_escribir);

        sem_wait(&semOkEscritura);

        log_info(logger_cpu, "PID: <%d> - Acción: <%s> - Dirección Física: <%u> - Valor: <%s>", contexto->pid, "ESCRIBIR" ,direccion_fisica, datos_a_escribir);
   
    }


    string_array_destroy(parte);

    return;
}


void instruccion_leer_memoria(char** parte)
{  
    // READ(Registro Direccion, Tamaño): 
    // Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el Registro Datos
    log_instruccion(parte);

     // Leo el tamaño que va a leer en memoria
    int tamanio = atoi(parte[2]); // parte[2] = "20"

    // Obtengo la direccion logica (registro direccion)
    int direccion_logica = atoi(parte[1]); // parte[1] = "0"


    if (cache_paginas != NULL && entradas_cache > 0) //Si la cache esta habilitada trabajo directamente en cache
        leer_cache(direccion_logica,tamanio);

    else //sino le pido la direccion lógica a memoria y leo en memoria
    {
        int direccion_fisica = traducir_direccion_logica(direccion_logica);

        if (direccion_fisica < 0) {
            log_error(logger_cpu, "Segmentation Fault al traducir la dirección lógica %d", direccion_logica);
            exit(1);
            return;
        }

        peticion_lectura_a_memoria(direccion_fisica, tamanio);

        sem_wait(&sem_valor_leido);

        log_info(logger_cpu, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%s>", contexto->pid, direccion_fisica, valor_leido_memoria);
    }
    string_array_destroy(parte);
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

    string_array_destroy(parte);

}

void peticion_escritura_a_memoria(int direccion_fisica, char* valor_registro_dato)
{
    t_paquete* paquete = crear_super_paquete(CPU_PIDE_ESCRIBIR_MEMORIA);
    cargar_int_al_super_paquete(paquete, contexto->pid);
    cargar_int_al_super_paquete(paquete, direccion_fisica);
    cargar_string_al_super_paquete(paquete, valor_registro_dato);
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);
    
}


int buscar_en_tlb(int direccion_logica) {
    int nro_pagina = direccion_logica / tamanio_pagina;
    int desplazamiento = direccion_logica % tamanio_pagina;

    if (entradas_tlb <= 0) {
    return -1;
    }

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
    
    return -1;
}

void agregar_a_tlb(int pid, int nro_pagina, int nro_marco) {
    sem_wait(&mutex_lista_tlb);

    EntradaTLB* nueva_entrada = malloc(sizeof(EntradaTLB));
    nueva_entrada->pid = pid;
    nueva_entrada->nro_pagina = nro_pagina;
    nueva_entrada->nro_marco = nro_marco;
    nueva_entrada->timestamp = obtener_timestamp_actual(); 


    if (list_size(lista_tlb) >= entradas_tlb) {
        if (strcmp(reemplazo_tlb, "FIFO") == 0) {
            if (list_size(lista_tlb) > 0) {
                EntradaTLB* entrada_removida = list_remove(lista_tlb, 0);
                log_info(logger_cpu, "Reemplazo FIFO - Eliminando PID: %d Página: %d", entrada_removida->pid, entrada_removida->nro_pagina);
                free(entrada_removida);
            }
        } else if (strcmp(reemplazo_tlb, "LRU") == 0) {
            int indice_mas_viejo = 0;
            long ts_min = ((EntradaTLB*)list_get(lista_tlb, 0))->timestamp;

            for (int i = 1; i < list_size(lista_tlb); i++) {
                EntradaTLB* entrada = list_get(lista_tlb, i);
                if (entrada->timestamp < ts_min) {
                    ts_min = entrada->timestamp;
                    indice_mas_viejo = i;
                }
            }
            int size = list_size(lista_tlb);
            if (indice_mas_viejo >= size) {
                log_error(logger_cpu, "ERROR: índice fuera de rango en list_remove. indice=%d, size=%d", indice_mas_viejo, size);
                return;
            }
            
            EntradaTLB* entrada_removida = list_remove(lista_tlb, indice_mas_viejo);
            log_info(logger_cpu, "Reemplazo LRU - Eliminando PID: %d Página: %d", entrada_removida->pid, entrada_removida->nro_pagina);
            free(entrada_removida);
        }
    }

    list_add(lista_tlb, nueva_entrada);
    log_info(logger_cpu, "Agregando a TLB: PID: %d Página: %d Marco: %d", pid, nro_pagina, nro_marco);

    sem_post(&mutex_lista_tlb);
}

int obtener_timestamp_actual() {
    return timestamp_actual++;// ACTUALIZO UN TIMESTAMP GLOBAL, para que cambie y siempre sea uno mayor que otro. 
}


int traducir_direccion_logica(int direccion_logica) {

    int direccion_fisica;

    direccion_fisica = buscar_en_tlb(direccion_logica);
    if (direccion_fisica != -1) 
    {
        return direccion_fisica;// Si es HIT devuelve, y no sigue con la MMU
    }

    //Si no esta esta en la tlb traduce mediante la mmu
    direccion_fisica = traducirDLMedianteMMU(direccion_logica);

    return direccion_fisica;
}

int traducirDLMedianteMMU(int direccion_logica)
{
    int nro_pagina = direccion_logica / tamanio_pagina;
    int desplazamiento = direccion_logica % tamanio_pagina;
    int entrada_nivel_X;
    nro_marco = -1;
    int* entradas_de_nivel = malloc(sizeof(int)* (cant_niveles + 1));

    for (int nivel = 1; nivel <= cant_niveles; nivel++) {
        int potencia = (int) pow(cant_entradas_tabla, cant_niveles - nivel);
        entrada_nivel_X = ((nro_pagina / potencia) % cant_entradas_tabla);

        if (nivel < cant_niveles) {
            entradas_de_nivel[nivel] = entrada_nivel_X;
            solicitar_tabla_a_memoria(); //SIMULO LA SOLICITUD DE TABLA, ENREALIDAD LE SOLICITO TODAS AL FINAL
        } else {
            entradas_de_nivel[nivel] = entrada_nivel_X;
            for (int i = 1; i < cant_niveles; i++)
            solicitar_marco_a_memoria(entradas_de_nivel); 
        }
    }

    free(entradas_de_nivel);

        // Verificar si nro_marco es válido antes de calcular la dirección física
    if (nro_marco < 0) {
        log_error(logger_cpu, "Error: nro_marco es inválido. No se puede calcular la dirección física.");
        return -1; // Manejo de error
    }

    // Calculamos dirección física
    int direccion_fisica = nro_marco * tamanio_pagina + desplazamiento;

    log_info(logger_cpu, "## PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", contexto->pid, nro_pagina, nro_marco);
    //Agregar a TLB

    if (entradas_tlb != 0) {
        agregar_a_tlb(contexto->pid, nro_pagina, nro_marco); 
    } 
    
    return direccion_fisica;
}


void solicitar_tabla_a_memoria()
{
    //TODAVIA NO LE MANDO LA ENTRADA_X PQ CUANDO LE PIDA LA DIRECCION FISICA LE MANDO TODAS LAS ENTRADAS JUNTAS, LO UNICO LE MANDO EL OPCODE PARA SIMULAR EL RETARDO DE LA MEMORIA
    enviarOpCode(socket_cpu_memoria,SOLICITUD_TABLA);
    sem_wait(&semLlegoPeticionTabla);
    
}

void solicitar_marco_a_memoria(int* entradas_de_nivel)
{
    t_paquete* paquete = crear_super_paquete(SOLICITUD_FRAME);
    cargar_uint32_t_al_super_paquete(paquete,contexto->pid);
    
    for (int nivel = 1; nivel <= cant_niveles; nivel++)
    {
        cargar_int_al_super_paquete(paquete,entradas_de_nivel[nivel]); //Cargo todas las entradas de nivel previamente calculadas
    }
    enviar_paquete(paquete,socket_cpu_memoria);
    eliminar_paquete(paquete);
    sem_wait(&semLlegoPeticionMMU);//Quiere decir que ya cargo el marco que le llego
}

void peticion_lectura_a_memoria(int direccion_fisica, int tamanio)
{
    t_paquete* paquete = crear_super_paquete(CPU_PIDE_LEER_MEMORIA);
    cargar_int_al_super_paquete(paquete, contexto->pid);
    cargar_int_al_super_paquete(paquete, direccion_fisica);
    cargar_int_al_super_paquete(paquete, tamanio);
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);
}


// --------------- ESTO ES FETCH --------------- //// EJEMPLO CPU_PIDE_INSTRUCCION_A_MEMORIA

void fetch(int socket_cpu_memoria) 
{   
    // Obtener instrucción
    // Deberemos devolverle la instrucción correspondiente pid y al Program Counter recibido. 
    
    sem_wait(&semFetch);

    sem_wait(&semContextoCargado);

    t_paquete* paquete;

    // CASO EN EL QUE SE EJECUTA  SOLAMENTE UNA INSTRUCCION COMUN Y SE VUELVE A SOLICITAR OTRA
    paquete = crear_super_paquete(CPU_PIDE_INSTRUCCION_A_MEMORIA);
    

    cargar_uint32_t_al_super_paquete(paquete, contexto->pid);
    cargar_uint32_t_al_super_paquete(paquete, contexto->registros.PC);
    log_info(logger_cpu, "## PID: %d - FETCH - Program Counter: %d", contexto->pid, contexto->registros.PC);
    

    // Envio el paquete a memoria
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);
}

void decode()
{     
        
        sem_wait(&sem_hay_instruccion); 
        
       
       
        char* instruccionRecibidaLocal = strdup(instruccion_recibida);
       
        char** parte = string_split(instruccionRecibidaLocal, " "); // Divido la instrucción (que es un string) en partes  (decode)

        free(instruccionRecibidaLocal);

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
                //destruir_diccionarios();
                    return;
            default:
                log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
                break;
            }
}
     


void check_interrupt(uint32_t PIDInicial)
{   
    //El semaforo de recibir interrupcion no es necesario pq la validacion de que ya llego una posible interrupcion para este punto la ahce kernel antes de mandar el OK en las syscalls
    sem_wait(&mutex_motivo_interrupcion);
    bool hay_interrupcion = flag_interrupcion;  // Leer flag
    sem_post(&mutex_motivo_interrupcion);

    if(hay_interrupcion) // hay una interrupcion
    {   
        
        free(instruccion_recibida);
        instruccion_recibida = NULL;
        
        sem_wait(&semMutexContexto);
            


            if(cache_paginas!=NULL)
                desalojar_proceso_de_cache();
            
            if(lista_tlb != NULL)
            {
                liberar_entradas_tlb();
            }

            
        sem_wait(&mutex_motivo_interrupcion);
            flag_interrupcion = false;  // Leer flag
            if(motivo_interrupcion == INTERRUPCION_ASINCRONICA)
            {
                log_debug(logger_cpu,"<%u>enviando PC=%u ACTUALIZADO",contexto->pid,contexto->registros.PC);
                t_paquete* paquete = crear_super_paquete(PC_INTERRUPCION_ASINCRONICA);
                cargar_uint32_t_al_super_paquete(paquete,contexto->pid);
                cargar_uint32_t_al_super_paquete(paquete,contexto->registros.PC);
                enviar_paquete(paquete,socket_cpu_kernel_dispatch);
                eliminar_paquete(paquete);
                enviarOpCode(socket_cpu_kernel_dispatch, EN_CHECK_INTERRUPT);
            }
        sem_post(&mutex_motivo_interrupcion); 


        sem_post(&semMutexContexto);

        sem_post(&semFinCicloInstruccion);
        
        sem_post(&semFetch); //Si todavia no se recibio el nuevo PID a ejecutar , solo hay 1/2 semaforos necerios para volver a arrancar el ciclo de instrucion

        
        
        
        

    } else {

        free(instruccion_recibida);
        instruccion_recibida = NULL;
        
        log_debug(logger_cpu,"--------------No hay interrupcion \n");
        
        
        sem_post(&semFetch); // 1/2 semaforos necesarios para que vuelva a empezar el ciclo de instruccion
        sem_post(&semContextoCargado); // 2/2 semaforos para que vuelva a empezar el ciclo de instruccion
    }
}

void ciclo_instruccion(int socket_cpu_memoria)
{
    //sem_wait(&semMutexContexto); // Para que en srt no cambie contexto a mitad de la ejecución de la instrucción
        fetch(socket_cpu_memoria);
        
        uint32_t pidInicial = contexto->pid;
        
        
        contexto->registros.PC = contexto->registros.PC + 1;
        
        decode();
    //sem_post(&semMutexContexto);
    
    check_interrupt(pidInicial);

    

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



void inicializar_cache() 
{
    if (entradas_cache <= 0) {
        log_error(logger_cpu, "Entradas de caché inválidas: %d", entradas_cache);
        cache_paginas = NULL;
        return;
    }

    if (tamanio_pagina <= 0) {
        log_error(logger_cpu, "Tamaño de página inválido: %d", tamanio_pagina);
        cache_paginas = NULL;
        return;
    }

    cache_paginas = malloc(sizeof(EntradaCache) * entradas_cache);
    puntero_clock = 0;

    for (int i = 0; i < entradas_cache; i++) {
        cache_paginas[i].pid = -1;
        cache_paginas[i].nro_pagina = -1;
        cache_paginas[i].nro_marco = -1;
        cache_paginas[i].contenido = malloc(tamanio_pagina);
        if (cache_paginas[i].contenido != NULL) {
            memset(cache_paginas[i].contenido, 0, tamanio_pagina);
        }
        cache_paginas[i].bit_referencia = false;
        cache_paginas[i].bit_modificacion = false;
        cache_paginas[i].bit_validez = false;
    }
}

int buscar_en_cache(int pid, int nro_pagina) 
{   

    for (int i = 0; i < entradas_cache; i++) {
        if (cache_paginas[i].bit_validez != false && 
            cache_paginas[i].pid == pid && 
            cache_paginas[i].nro_pagina == nro_pagina) {
            
            //CACHE HIT
            cache_paginas[i].bit_referencia = true;
            log_info(logger_cpu, "PID: <%d> - CACHE HIT - Página: <%d>", pid, nro_pagina);
            return i; 
        }
    }
    //CACHE MISS
    log_info(logger_cpu, "PID: <%d> - CACHE MISS - Página: <%d>", pid, nro_pagina);
    return -1; // No encontrada
}

int algoritmo_clock() 
{
    
    for (int i = 0; i < entradas_cache; i++) {
        if (cache_paginas[puntero_clock].bit_referencia == false) {
            int victima = puntero_clock;
            puntero_clock = (puntero_clock + 1) % entradas_cache; //avanzo
            return victima;
        }

        cache_paginas[puntero_clock].bit_referencia = false; //si es true le da otra vuelta
        puntero_clock = (puntero_clock + 1) % entradas_cache; //avanzo
    }
    
    //si todas estaban en true, ahora todas tienen false y la victima es el primero
    int victima = puntero_clock;
    puntero_clock = (puntero_clock + 1) % entradas_cache;

    return victima;
}

int algoritmo_clock_modificado() 
{
    
    //Primera vuelta:buscar (0,0)
    for(int i = 0; i < entradas_cache; i++) {
        if (cache_paginas[puntero_clock].bit_referencia == false && 
            cache_paginas[puntero_clock].bit_modificacion == false) {
            int victima = puntero_clock;
            puntero_clock = (puntero_clock + 1) % entradas_cache;
            return victima;
        }
        puntero_clock = (puntero_clock + 1) % entradas_cache;
    }
    
    //Segunda vuelta:buscar (0,1)
    for(int i = 0; i < entradas_cache; i++) {
        if (cache_paginas[puntero_clock].bit_referencia == false) {
            int victima = puntero_clock;
            puntero_clock = (puntero_clock + 1) % entradas_cache;
            return victima;
        }
        cache_paginas[puntero_clock].bit_referencia = false;
        puntero_clock = (puntero_clock + 1) % entradas_cache;
    }
    
    // Si llega acá, usa la posición actual
    int victima = puntero_clock;
    puntero_clock = (puntero_clock + 1) % entradas_cache;
    return victima;
}

int seleccionar_victima() 
{
    // Buscar entrada libre 
    for (int i = 0; i < entradas_cache; i++) {
        if (cache_paginas[i].bit_validez == false) {
            return i;
        }
    }

    // No hay entradas libres, aplico algoritmo 
    if (strcmp(reemplazo_cache, "CLOCK") == 0) {
        return algoritmo_clock();
    } else if (strcmp(reemplazo_cache, "CLOCK-M") == 0) {
        return algoritmo_clock_modificado();
    } else {
        return 0; 
    }
}

void escribir_pagina_a_memoria(int indice_cache) 
{
    if (cache_paginas[indice_cache].bit_modificacion == false) {
        return; 
    }
    
    EntradaCache* entrada = &cache_paginas[indice_cache];
    int direccion_fisica = entrada -> nro_marco * tamanio_pagina;
    
    // Enviar página completa a memoria
    t_paquete* paquete = crear_super_paquete(CPU_SOLICITA_ESCRIBIR_PAGINA_COMPLETA); 
    cargar_int_al_super_paquete(paquete, entrada->pid);
    cargar_int_al_super_paquete(paquete, direccion_fisica);
    cargar_string_al_super_paquete(paquete, entrada->contenido); // Página completa
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);

    sem_wait(&sem_pagina_escrita);

    log_info(logger_cpu, "PID: <%d> - Memory Update - Página: <%d> - Frame: <%d>", entrada->pid, entrada->nro_pagina, entrada->nro_marco);
    
    entrada->bit_modificacion = false;
}

int cargar_pagina_en_cache(int pid, int direccion_logica) 
{
    if (cache_paginas == NULL){
        return NULL;
    }

    int indice_victima = seleccionar_victima();
    
    // Si hay una página válida en esa posición, escribirla si está modificada
    // Se mantiene todo actualizado
    if (cache_paginas[indice_victima].bit_validez != false) {
        escribir_pagina_a_memoria(indice_victima);
    }
    

    
    // Cargo nueva página desde memoria
    int direccion_fisica = traducir_direccion_logica(direccion_logica);

    if (direccion_fisica < 0) {
        log_error(logger_cpu, "Segmentation Fault al traducir la dirección lógica %d", direccion_logica);
        exit(1);
        return;
        }
    int nro_pagina = direccion_logica / tamanio_pagina;
        
    // Solicito contenido de la página a memoria
    t_paquete* paquete = crear_super_paquete(CPU_SOLICITA_LEER_PAGINA_COMPLETA); 
    cargar_int_al_super_paquete(paquete, pid);
    cargar_int_al_super_paquete(paquete, direccion_fisica);
    cargar_int_al_super_paquete(paquete, tamanio_pagina);
    enviar_paquete(paquete, socket_cpu_memoria);
    eliminar_paquete(paquete);
    
    
    sem_wait(&sem_pagina_recibida); // Espera a que el hilo de atender_memoria devuelva la página
    

    
    // Actualizo entrada de caché
    cache_paginas[indice_victima].pid = pid;
    cache_paginas[indice_victima].nro_pagina = nro_pagina;
    cache_paginas[indice_victima].nro_marco = nro_marco;
    
    if (buffer_pagina_recibida != NULL) {
        memcpy(cache_paginas[indice_victima].contenido, buffer_pagina_recibida, tamanio_pagina);
        free(buffer_pagina_recibida); // Liberar el buffer temporal
        buffer_pagina_recibida = NULL; // Resetear el puntero
    } else {
        memset(cache_paginas[indice_victima].contenido, 0, tamanio_pagina); // Limpiar para evitar basura
    }

    cache_paginas[indice_victima].bit_referencia = true;
    cache_paginas[indice_victima].bit_modificacion = false;
    cache_paginas[indice_victima].bit_validez = true;



    //log_info(logger_cpu, "PID: %d - Cache Add - Página: %d", pid, nro_pagina);

         /*for (int i = 0; i < entradas_cache; i++) {
    log_debug(logger_cpu, "[DEBUG] Entrada %d -> PID: %d | Página: %d | Marco: %d | Validez: %d | Uso: %d | Modif: %d",
              i,
              cache_paginas[i].pid,
              cache_paginas[i].nro_pagina,
              cache_paginas[i].nro_marco,
              cache_paginas[i].bit_validez,
              cache_paginas[i].bit_referencia,
              cache_paginas[i].bit_modificacion);
}*/



    return indice_victima;
    
}

void escribir_cache(int direccion_logica, char *valor) // Chequear siempre si cache no esta en NULL antes de llamar a esta funcion
{

    int nro_pagina = direccion_logica / tamanio_pagina;
    int desplazamiento = direccion_logica % tamanio_pagina;

    char *valor_escrito = malloc(strlen(valor) + 1);

    int indice_cache = buscar_en_cache(contexto->pid, nro_pagina);
    if (indice_cache == -1) // Si no esta en cache, la cargo
        indice_cache = cargar_pagina_en_cache(contexto->pid, direccion_logica);

    // escribo el valor en cache
    strcpy(cache_paginas[indice_cache].contenido + desplazamiento, valor);
    strcpy(valor_escrito, valor);

    cache_paginas[indice_cache].bit_modificacion = true;

    int direccion_fisica_log = cache_paginas[indice_cache].nro_marco * tamanio_pagina + desplazamiento;
    // char valor_str[2] = {valor, '\0'}; //no se para que era esto
    log_info(logger_cpu, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%u> - Valor: <%s>",
             contexto->pid, direccion_fisica_log, valor_escrito);

    free(valor_escrito);
}

void leer_cache(int direccion_logica, int tamanio) {
    int nro_pagina = direccion_logica / tamanio_pagina;
    int desplazamiento = direccion_logica % tamanio_pagina;
    char* valor_leido_cache = malloc(tamanio+1);
    
    int indice_cache = buscar_en_cache(contexto->pid, nro_pagina);
    if (indice_cache == -1) 
        indice_cache = cargar_pagina_en_cache(contexto->pid, direccion_logica);
    strncpy(valor_leido_cache,cache_paginas[indice_cache].contenido + desplazamiento, tamanio);
    
    valor_leido_cache[tamanio] = '\0';

    int direccion_fisica_log = cache_paginas[indice_cache].nro_marco * tamanio_pagina + desplazamiento;
    log_info(logger_cpu, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%s>", contexto->pid, direccion_fisica_log, valor_leido_cache);
    free(valor_leido_cache);

}



void escribir_paginas_modificadas_proceso() 
{
    
    for (int i = 0; i < entradas_cache; i++) {
        if (cache_paginas[i].bit_validez != false && 
            cache_paginas[i].bit_modificacion != false) {
            
            escribir_pagina_a_memoria(i);
        }
    }
}

void eliminar_paginas_proceso_de_cache() 
{
    for (int i = 0; i < entradas_cache; i++) {
        if (cache_paginas[i].bit_validez != false) {
            cache_paginas[i].bit_validez = false;
            cache_paginas[i].pid = -1;
            cache_paginas[i].nro_pagina = -1;
            cache_paginas[i].nro_marco = -1;
            cache_paginas[i].bit_referencia = false;
            cache_paginas[i].bit_modificacion = false;
            memset(cache_paginas[i].contenido, 0, tamanio_pagina);
        }
    }
}

void desalojar_proceso_de_cache() 
{
    escribir_paginas_modificadas_proceso();
    eliminar_paginas_proceso_de_cache();
}


void liberar_entradas_tlb()
{
    
    list_clean_and_destroy_elements(lista_tlb, destruir_entrada_tlb);
}

void destruir_entrada_tlb(void* entrada) {
    free(entrada); 
}

void liberar_cache()
{
    for (int i = 0; i < entradas_cache; i++) {
        free(cache_paginas[i].contenido);  // liberar contenido de cada entrada
    }
    free(cache_paginas);  // liberar el array de estructuras
}




