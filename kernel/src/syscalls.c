#include "syscalls.h"
#include "conexionConMemoria.h"


void INIT_PROC(char* archivoPseudocodigo,uint32_t tam){
    //Creo un nuevo proceso
    PCB* nuevoProceso=malloc(sizeof(PCB));
    nuevoProceso->archivoPseudocodigo=strdup(archivoPseudocodigo);
    nuevoProceso->tam=tam;
    nuevoProceso->PC=0;
    
    sem_wait(semaforoMutexPIDDisponible);
        nuevoProceso->PID=pidDisponible;
        pidDisponible++;
    sem_post(semaforoMutexPIDDisponible);

    

    for(int i=0;i<7;i++)
    {
        nuevoProceso->ME[i]=0;
        nuevoProceso->MT[i]=0;
        nuevoProceso->cronometros[i]=temporal_create();
        temporal_stop(nuevoProceso->cronometros[i]);
    }

    nuevoProceso->semMutex =malloc(sizeof(sem_t));
    sem_init(nuevoProceso->semMutex,1,1);

    
    nuevoProceso->estimadoSiguienteRafaga=estimacion_inicial;

    
   
    if(algoritmoColaNewEnFIFO)
        agregarALista(listaProcesosNew,nuevoProceso);
    else
        agregarAListaOrdenada(listaProcesosNew,nuevoProceso,menorTam);

    temporal_resume(nuevoProceso->cronometros[NEW]);
    nuevoProceso->ME[NEW]++; 

    log_info(loggerKernel, "## (<%u>) Se crea el proceso - Estado: NEW",nuevoProceso->PID);
    
    if(nuevoProceso->PID==0) //Si es el primer proceso, espero el ENTER
    {
        while (1) {

            char* input = readline("Apriete ENTER para empezar a planificar procesos.\n");  

            if (*input == '\0') {  
                break;
            }

            free(input);
            
        }
    }    
    
    inicializarProceso();
}

void dump_memory(uint32_t pid) {
    log_info(loggerKernel, "## (<%u>) - Solicitó syscall: DUMP_MEMORY", pid);
    
    PCB* proceso = buscarPCBEjecutando(pid);
    if (proceso == NULL) {
        log_error(loggerKernel, "## (<%u>) - No se encontró el PCB para syscall DUMP_MEMORY", pid);
        exit(1);
    }
    
    ProcesoEnEsperaDump* procesoEsperandoDump = malloc(sizeof(ProcesoEnEsperaDump));
    procesoEsperandoDump->proceso = proceso;
    procesoEsperandoDump->semaforoDumpFinalizado = malloc(sizeof(sem_t));
    procesoEsperandoDump->semaforoMutex = malloc(sizeof(sem_t));
    sem_init(procesoEsperandoDump->semaforoDumpFinalizado, 1, 0);
    sem_init(procesoEsperandoDump->semaforoMutex, 1, 1);
    
    char* PIDComoChar = pasarUnsignedAChar(pid);
    agregarADiccionario(diccionarioProcesosEsperandoDump, PIDComoChar, procesoEsperandoDump);
    free(PIDComoChar);
    
    if (solicitar_dump_memoria(pid)) {
        log_info(loggerKernel, "## (<%u>) - Memory Dump solicitado, esperando confirmación", pid);
        
        sem_wait(procesoEsperandoDump->semaforoDumpFinalizado);
        
        char* PIDComoChar2 = pasarUnsignedAChar(pid);
        sacarDeDiccionario(diccionarioProcesosEsperandoDump, PIDComoChar2);
        free(PIDComoChar2);
        
        free(procesoEsperandoDump->semaforoDumpFinalizado);
        free(procesoEsperandoDump->semaforoMutex);
        free(procesoEsperandoDump);
        
        log_info(loggerKernel, "## (<%u>) - Memory Dump completado", pid);
    } else {
        log_error(loggerKernel, "## (<%u>) - Error al solicitar Memory Dump", pid);
        
        char* PIDComoChar2 = pasarUnsignedAChar(pid);
        sacarDeDiccionario(diccionarioProcesosEsperandoDump, PIDComoChar2);
        free(PIDComoChar2);
        
        free(procesoEsperandoDump->semaforoDumpFinalizado);
        free(procesoEsperandoDump->semaforoMutex);
        free(procesoEsperandoDump);
    }
}

void syscall_IO(uint32_t pid, char* nombreIO, int64_t tiempo) {
    log_info(loggerKernel, "## (<%u>) - Solicitó syscall: IO", pid);

    PCB* proceso = NULL;
    proceso  = buscarPCBEjecutando(pid);

    if (proceso == NULL) {
        log_error(loggerKernel, "## (<%u>) - No se encontró el PCB para syscall IO", pid);
        exit(1);
    }
    
    
    
   



    DispositivoIO* dispositivo = NULL;
    dispositivo = leerDeDiccionario(diccionarioDispositivosIO,nombreIO);

    if (dispositivo == NULL) {
        log_error(loggerKernel, "## (<%u>) - Dispositivo IO %s no encontrado. Finalizando proceso", pid, nombreIO);
        terminarEjecucion(proceso,INTERRUPCION_SINCRONICA);
        pasarAExit(proceso,"EXECUTE");
        return;
    }

    log_info(loggerKernel, "## (<%u>) - Bloqueado por IO: <%s>",pid,nombreIO);

    terminarEjecucion(proceso,INTERRUPCION_SINCRONICA);
    pasarABLoqueadoPorIO(proceso, tiempo, nombreIO);
    
}

void syscallExit(uint32_t pid)
{
    PCB* proceso = NULL;
    proceso = buscarPCBEjecutando(pid);

    if (proceso == NULL) {
        log_error(loggerKernel, "## (<%u>) - No se encontró el PCB para syscall Exit", pid);
        exit(1);
    }

    
    terminarEjecucion(proceso,INTERRUPCION_SINCRONICA);
    pasarAExit(proceso,"EXECUTE");
    
    
    

}
PCB* buscarPCBEjecutando(uint32_t pid) {
    bool _mismoPID(NucleoCPU* nucleoEnEjecucion) {
        return nucleoEnEjecucion->procesoEnEjecucion->PID == pid;
    };

    bool _mismoPID2(PCB* procesoEnEjecucion) {
        return procesoEnEjecucion->PID == pid;
    };

    NucleoCPU* NucleoCPU = leerDeListaSegunCondicion(listaCPUsEnUso,_mismoPID);
    PCB* procesoPorSerDesalojado = leerDeListaSegunCondicion(listaProcesosPorSerDesalojados,_mismoPID2);

    if(NucleoCPU!= NULL)
        return NucleoCPU->procesoEnEjecucion;
    else if(procesoPorSerDesalojado!=NULL)
        return procesoPorSerDesalojado;
    else
        return NULL;
}

