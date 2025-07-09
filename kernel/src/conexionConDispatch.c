#include "conexionConCPU.h"
#include "../../utils/src/utils/conexiones.h"

void* esperarClientesDispatch(void* arg)
{
    while(1)
    {
        int* fdConexion = malloc(sizeof(int));
        *fdConexion = esperar_cliente(socket_kernel_cpu_dispatch);
        log_info(loggerKernel, "## Se conectó DISPATCH");
        pthread_t hilo_dispatch_cpu;
        pthread_create(&hilo_dispatch_cpu, NULL, (void*) atender_dispatch_cpu, fdConexion);
    }
}

void atender_dispatch_cpu(void* conexion) 
{
    int fdConexion = *(int*)conexion;
    free(conexion);
    
    
    int cod_op;
    NucleoCPU* NucleoCPU;
    uint32_t PID;
    uint32_t PC;
    t_buffer* buffer;
    
    
   
    
    while(1) 
    {
        cod_op = recibir_operacion(fdConexion);
        switch (cod_op) 
        {
            case 1:
                sem_post(semaforoEsperarOKDispatch);
                break;
            case HANDSHAKE_CPU_KERNEL_D:
                buffer = recibiendo_super_paquete(fdConexion);
                char* identificador = recibir_string_del_buffer(buffer);    
                NucleoCPU = guardarDatosCPUDispatch(identificador,fdConexion);
                limpiarBuffer(buffer);
                break;

            case PC_INTERRUPCION_ASINCRONICA:
                buffer = recibiendo_super_paquete(fdConexion);
                PID = recibir_uint32_t_del_buffer(buffer);
                PC = recibir_uint32_t_del_buffer(buffer);
                actualizarPCAsincronico(PID,PC);
                //sem_post(semaforoPCActualizado);
                break;
            case IO:
                buffer = recibiendo_super_paquete(fdConexion);
                PID = recibir_uint32_t_del_buffer(buffer);
                PC = recibir_uint32_t_del_buffer(buffer);
                char* nombreIO = recibir_string_del_buffer(buffer);
                int64_t tiempoEnIO = recibir_int64_t_del_buffer(buffer);
                actualizarPC(PID,PC);
                PCB* proceso  = buscarPCBEjecutando(PID);
                terminarEjecucion(proceso,INTERRUPCION_SINCRONICA);
                enviarOK(fdConexion);
                syscall_IO(proceso,PID,nombreIO,tiempoEnIO);
                free(nombreIO);
                limpiarBuffer(buffer);              
                break;
            
            case DUMP_MEMORY:
                buffer = recibiendo_super_paquete(fdConexion);
                PID = recibir_uint32_t_del_buffer(buffer);
                PC = recibir_uint32_t_del_buffer(buffer);
                actualizarPC(PID,PC);
                dump_memory(PID);
                enviarOK(fdConexion);
                limpiarBuffer(buffer);
                break;
            
            case INIT_PROCCESS:
                buffer = recibiendo_super_paquete(fdConexion);
                PID = recibir_uint32_t_del_buffer(buffer);
                char* nombrePseudocodigo = recibir_string_del_buffer(buffer);
                uint32_t tam = recibir_uint32_t_del_buffer(buffer);
                INIT_PROC(nombrePseudocodigo,tam);
                free(nombrePseudocodigo);
                enviarOK(fdConexion);
                limpiarBuffer(buffer);
                break;
            
            case SYSCALL_EXIT:
                buffer = recibiendo_super_paquete(fdConexion);
                PID = recibir_uint32_t_del_buffer(buffer);
                PC = recibir_uint32_t_del_buffer(buffer);
                proceso = NULL;
                proceso = buscarPCBEjecutando(PID);
                if (proceso == NULL) {
                    log_error(loggerKernel, "## (<%u>) - No se encontró el PCB para syscall Exit", PID);
                    exit(1);
                }
                terminarEjecucion(proceso,INTERRUPCION_SINCRONICA);
                enviarOK(fdConexion); 
                pasarAExit(proceso,"EXECUTE");
                limpiarBuffer(buffer);     
                break;
            case EN_CHECK_INTERRUPT:
                sem_post(semaforoEnCheckInterrupt);
                break;
            
            case -1:
                log_info(loggerKernel, "KERNEL DISPATCH se desconecto. Terminando servidor");
                //shutdown(fdConexion, SHUT_RDWR);
                close(fdConexion);
                pthread_exit(NULL);
            default:
                log_warning(loggerKernel, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}



NucleoCPU* guardarDatosCPUDispatch(char* identificador,int fdConexion)
{
    sem_wait(semaforoMutexGuardarDatosCPU);
    
    NucleoCPU* NucleoCPU = chequearSiCPUYaPuedeInicializarse(identificador);
    if(NucleoCPU == NULL)//Si esta en NULL quiere decir que la otra conexion todavía no llego
    {
        NucleoCPU = malloc(sizeof(*NucleoCPU));
        NucleoCPU->identificador= malloc(strlen(identificador)+1);
        strcpy(NucleoCPU->identificador,identificador);
        free(identificador);
       NucleoCPU->procesoEnEjecucion=NULL;
        NucleoCPU->fdConexionDispatch = fdConexion;
        agregarALista(listaCPUsAInicializar,NucleoCPU);
    }
    else 
    {
        sacarElementoDeLista(listaCPUsAInicializar,NucleoCPU);
        agregarALista(listaCPUsLibres,NucleoCPU);
        NucleoCPU->fdConexionDispatch = fdConexion;
        sem_post(semaforoIntentarPlanificar);
    }

    sem_post(semaforoMutexGuardarDatosCPU);

    return NucleoCPU;
}




NucleoCPU* chequearSiCPUYaPuedeInicializarse(char* identificador)
{
bool _mismoIdentificador(NucleoCPU* NucleoCPU)
{
    return (strcmp(NucleoCPU->identificador,identificador) == 0);
};

return sacarDeListaSegunCondicion(listaCPUsAInicializar,_mismoIdentificador);
}



void pasarAExit(PCB* proceso,char* estadoActual){
    log_info(loggerKernel,"## (<%u>) - Finaliza el proceso",proceso->PID);
    log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",proceso->PID,estadoActual,"EXIT");
    proceso->ME[EXIT]++;
    loggearMetricas(proceso);
    hacerFreeDeCronometros(proceso);
    avisarFinDeProcesoAMemoria(proceso->PID);
    sem_post(semaforoInicializarProceso);
    free(proceso->archivoPseudocodigo);
    sem_destroy(proceso->semMutex);
    free(proceso->semMutex);
    free(proceso);
}

void loggearMetricas(PCB* proceso)
{   
    log_info(loggerKernel,"## (<%u>) - Métricas de estado: NEW (%d) (%d), READY (%d) (%d) EXECUTE (%d) (%d) BLOCKED (%d) (%d) SWAP_BLOCKED (%d) (%d) SWAP_READY (%d) (%d) EXIT (%d) (%d)",
    proceso->PID,
    proceso->ME[NEW],proceso->MT[NEW],
    proceso->ME[READY],proceso->MT[READY],
    proceso->ME[EXECUTE],proceso->MT[EXECUTE],
    proceso->ME[BLOCKED],proceso->MT[BLOCKED],
    proceso->ME[SWAP_BLOCKED],proceso->MT[SWAP_BLOCKED],
    proceso->ME[SWAP_READY],proceso->MT[SWAP_READY], 
    proceso->ME[EXIT],proceso->MT[EXIT]);
}

void hacerFreeDeCronometros(PCB* proceso)
{
    for(int i = 0; i<7; i++)
    {
        temporal_destroy(proceso->cronometros[i]);
        
    }
}

void mandarContextoACPU(uint32_t PID,uint32_t PC,int fdConexion)
{
    t_paquete* paquete = crear_super_paquete(PID_KERNEL_A_CPU);
    cargar_uint32_t_al_super_paquete(paquete,PID);
    cargar_uint32_t_al_super_paquete(paquete,PC);
    enviar_paquete(paquete,fdConexion);
    //sem_wait(semaforoEsperarOKDispatch); //Para saber que cpu recibio el contexto
    eliminar_paquete(paquete);
}

void actualizarPC(uint32_t pid, uint32_t PCActualizado)
{
    PCB* proceso = buscarPCBEjecutando(pid);
    if(proceso!= NULL) 
        proceso->PC = PCActualizado; 

}

void actualizarPCAsincronico(uint32_t PID,uint32_t PCActualizado)
{
    log_debug(loggerKernel,"Actualizo PC asincrónico %u %u", PID, PCActualizado);

     bool _mismoPID(PCB* procesoEsperandoPC) {
        return procesoEsperandoPC->PID == PID;
    };
    
    PCB* proceso = sacarDeListaSegunCondicion(listaProcesosPorSerDesalojados,_mismoPID);
    if(proceso != NULL)// Puede ser que se este intentado desalojar un proceso que ya termino
        proceso->PC = PCActualizado;
}


NucleoCPU* buscarNucleoCPUPorPID(uint32_t PID)
{
    bool _ejecutandoProceso(NucleoCPU* nucleoCPU)
    {
        return nucleoCPU->procesoEnEjecucion != NULL && nucleoCPU->procesoEnEjecucion->PID == PID;
    };
    
    // Buscar en CPUs en uso
    NucleoCPU* nucleoCPU = leerDeListaSegunCondicion(listaCPUsEnUso, _ejecutandoProceso);
    if (nucleoCPU != NULL) {
        return nucleoCPU;
    }
    
    // Buscar en CPUs libres (por si acaso)
    nucleoCPU = leerDeListaSegunCondicion(listaCPUsLibres, _ejecutandoProceso);
    if (nucleoCPU != NULL) {
        return nucleoCPU;
    }
    
    // Buscar en CPUs a inicializar (último recurso)
    nucleoCPU = leerDeListaSegunCondicion(listaCPUsAInicializar, _ejecutandoProceso);
    return nucleoCPU;
} 