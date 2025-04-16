#include <syscalls.h>

void dump_memory()
{
	/*En este apartado solamente se tendrá la instrucción DUMP_MEMORY. Esta syscall le solicita a la memoria, junto al PID y TID que lo solicitó,
	que haga un Dump del proceso. Esta syscall bloqueará al hilo que la invocó hasta que el módulo memoria confirme la finalización de la operación,
	en caso de error, el proceso se enviará a EXIT. Caso contrario, el hilo se desbloquea normalmente pasando a READY.*/
	t_tcb *hiloEnExec;

    
    t_tcb *hiloAuxiliar = hiloEnExec;

	uint32_t tid, pid;

	tid = hiloAuxiliar->tid;
	pid = hiloAuxiliar->procesoPadre->pid;
	
	bool encontrarHiloPorTidPid(void *elemento)
	{
		t_tcb *unHilo = (t_tcb *)elemento;
		return unHilo->tid == tid && unHilo->procesoPadre->pid == pid;
	}

	log_info(logger_kernel, "## (<PID>:<TID>) - (<%d>:<%d>) - Solicitó syscall: <DUMP_MEMORY>",hiloAuxiliar->procesoPadre->pid, hiloAuxiliar->tid);

    int socket_memoria_particular = crear_conexion(config_get_string_value(config, "IP_MEMORIA"),config_get_string_value(config, "PUERTO_MEMORIA"),"MEMORIA");

    t_paquete* unPaquete = crear_paquete(MEMORY_DUMP);
    
	agregar_a_paquete(unPaquete, &pid, sizeof(uint32_t));
    agregar_a_paquete(unPaquete, &tid, sizeof(uint32_t));
    enviar_paquete(unPaquete, socket_memoria_particular);
    eliminar_paquete(unPaquete);

	list_add(hilosEnBlock, hiloAuxiliar);
	loggear_cambio_estado(hiloAuxiliar, BLOCKED, hiloAuxiliar->tid, true);
	hiloAuxiliar->estadoDelHilo = BLOCKED;

	log_info(logger_kernel, "## (<PID>:<TID>) - (<%d>:<%d>) - Bloqueado por: <DUMP>",hiloAuxiliar->procesoPadre->pid, hiloAuxiliar->tid);

	// Esperar respuesta de memoria
	op_code respuestaMemoria = recibir_operacion(socket_memoria_particular);
	
	char* respuesta = recibir_una_cadena(socket_memoria_particular);
	
    log_trace(logger_kernel,"Se cerro la conexion con MEMORIA en el socket: %d", socket_memoria_particular);
	
	close(socket_memoria_particular);
	
	if (respuestaMemoria == DUMP_MEMORY_OK)
	{
		log_trace(logger_kernel, "DUMP_MEMORY Respuesta de memoria recibida: %s", respuesta);
		
		pthread_mutex_lock(&mutex_Block);
		hiloAuxiliar = list_remove_by_condition(hilosEnBlock, encontrarHiloPorTidPid);
		pthread_mutex_unlock(&mutex_Block);

		if(!transicion_a_ready(hiloAuxiliar)){
			pthread_mutex_lock(&mutex_Block);
			list_add(hilosEnBlock, hiloAuxiliar);
			pthread_mutex_unlock(&mutex_Block);
		}
	}
	else
	{
		log_trace(logger_kernel, "DUMP_MEMORY Respuesta de memoria recibida: %s", respuesta);
		finalizar_proceso(hiloAuxiliar->procesoPadre);
	}

	free(respuesta);
}

void io()
{
	uint32_t tiempoBloqueado;
	recibir_un_entero(socket_cpu_dispatch, &tiempoBloqueado);

	t_solicitudIO *nuevaSolicitud = malloc(sizeof(t_solicitudIO));

	nuevaSolicitud->hiloAtendido = hiloEnExec;
	nuevaSolicitud->tiempoBloqueado = tiempoBloqueado;

	pthread_mutex_lock(&mutex_Block);
	list_add(hilosEnBlock, hiloEnExec);
	pthread_mutex_unlock(&mutex_Block);

	loggear_cambio_estado(hiloEnExec, BLOCKED, hiloEnExec->tid, true);
	log_info(logger_kernel, "## (<PID>:<TID>) - (<%d>:<%d>) - Bloqueado por: <IO>", hiloEnExec->procesoPadre->pid, hiloEnExec->tid);
	
	hiloEnExec->bloqueadoPorIO = true;
	hiloEnExec->estadoDelHilo = BLOCKED;

	pthread_mutex_lock(&mutex_io);
	list_add(dispositivoIO, nuevaSolicitud);
	pthread_mutex_unlock(&mutex_io);

	//log_info(logger, "Generada IO para el hilo (%d, %d) de %d", nuevaSolicitud->hiloAtendido->tid, nuevaSolicitud->hiloAtendido->procesoPadre->pid, nuevaSolicitud->tiempoBloqueado);

	sem_post(&semIO);
}

// Función para manejar la syscall IO
void syscall_io(PCB* proceso, char* dispositivo, int tiempo_ms) {
    // Bloquear el proceso
    sem_wait(&proceso->semaforo_estado);
    proceso->estado = BLOCKED;
    sem_post(&proceso->semaforo_estado);

    log_info(logger_kernel, "### (%d) - Bloqueado por IO: %s", proceso->PID, dispositivo);

    // Definir como se valida la existencia del dispositivo.
    //ListaDispositivoIO* disp = buscar_dispositivo_io(dispositivo);
    // if (disp == NULL) {
    //     log_error(logger_kernel, "Dispositivo %s no existe (PID: %d)", dispositivo, proceso->PID);
    //     terminar_proceso(proceso);
    //     return;
    // }

    // Enviar solicitud al módulo IO
    int socket_io = conectar_a_io(disp->ip, disp->puerto);
    if (socket_io == -1) {
        log_error(logger_kernel, "Error de conexión con IO %s (PID: %d)", dispositivo, proceso->PID);
        //terminar_proceso(proceso);
        return;
    }

    // Estructurar mensaje 
    t_buffer* buffer = crear_buffer();
    uint32_t operacion = OP_IO; // definir nombres de operaciones
    agregar_a_buffer(buffer, &operacion, sizeof(operacion));
    agregar_a_buffer(buffer, &proceso->PID, sizeof(proceso->PID));
    agregar_a_buffer(buffer, &tiempo_ms, sizeof(tiempo_ms));

    if (!enviar_mensaje(socket_io, buffer)) {
        log_error(logger_kernel, "Falló el envío a IO %s (PID: %d)", dispositivo, proceso->PID);
        terminar_proceso(proceso);
        liberar_buffer(buffer);
        close(socket_io);
        return;
    }

    // Iniciar timer de suspensión (hilo separado)
    pthread_t timer_thread;
    TimerArgs* args = malloc(sizeof(TimerArgs));
    args->proceso = proceso;
    args->tiempo_ms = tiempo_suspension;
    pthread_create(&timer_thread, NULL, manejar_tiempo_suspension, (void*)args);

    // Esperar respuesta (no bloqueante)
    registrar_callback_io(socket_io, proceso, dispositivo);

    liberar_buffer(buffer);
}

// Hilo para manejar tiempo de suspensión
void* manejar_tiempo_suspension(void* args) {
    TimerArgs* params = (TimerArgs*)args;
    usleep(params->tiempo_ms * 1000);

    sem_wait(&params->proceso->semaforo_estado);
    if (params->proceso->estado == BLOCKED) {
        // Mover a SUSP.BLOCKED y notificar a Memoria
        params->proceso->estado = SUSP_BLOCKED;
        log_info(logger_kernel, "### (%d) - Suspendido por timeout", params->proceso->PID);
        
        // Enviar a SWAP
        enviar_a_swap(params->proceso);
    }
    sem_post(&params->proceso->semaforo_estado);

    free(params);
    return NULL;
}

// Callback al recibir respuesta de IO
void registrar_callback_io(int socket_io, PCB* proceso, char* dispositivo) {
    // Implementación usando select() o poll()
    // Al recibir confirmación:
    sem_wait(&proceso->semaforo_estado);
    if (proceso->estado == BLOCKED) {
        proceso->estado = READY;
        log_info(logger_kernel, "### (%d) - Fin de IO en %s", proceso->PID, dispositivo);
        agregar_a_cola_ready(proceso);
    }
    sem_post(&proceso->semaforo_estado);
    close(socket_io);
}