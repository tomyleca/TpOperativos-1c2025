#include "syscalls.h"
#include "conexionConMemoria.h"
#include "../../utils/src/utils/conexiones.h"
#include "kernel.h"


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
    
   
    int valorSemaforo;
    sem_getvalue(semaforoInicializarProceso,&valorSemaforo);
    

    if(valorSemaforo<=3) 
        sem_post(semaforoInicializarProceso);
}

void dump_memory(PCB* proceso) {
    log_info(loggerKernel, "## (<%u>) - Solicitó syscall: DUMP_MEMORY", proceso->PID);
    

    
    ProcesoEnEsperaDump* procesoEsperandoDump = malloc(sizeof(ProcesoEnEsperaDump));
    procesoEsperandoDump->proceso = proceso;
    procesoEsperandoDump->semaforoDumpFinalizado = malloc(sizeof(sem_t));
    procesoEsperandoDump->semaforoMutex = malloc(sizeof(sem_t));
    procesoEsperandoDump->nucleoCPU = NULL;
    sem_init(procesoEsperandoDump->semaforoDumpFinalizado, 1, 0);
    sem_init(procesoEsperandoDump->semaforoMutex, 1, 1);

    
    //Lo paso a bloqueado
    char* PIDComoChar = pasarUnsignedAChar(proceso->PID);
    agregarADiccionario(diccionarioProcesosEsperandoDump, PIDComoChar, procesoEsperandoDump);
    log_info(loggerKernel, "## (<%u>) - Bloqueado por DUMP_MEMORY", proceso->PID);
    log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",proceso->PID,"EXECUTE","BLOCKED");
    proceso->ME[BLOCKED]++;
    temporal_resume(proceso->cronometros[BLOCKED]);
    free(PIDComoChar);
    
    pthread_t hiloEsperarDump;
    pthread_create(&hiloEsperarDump, NULL, (void*)manejarProcesoEsperandoDump, procesoEsperandoDump);
    pthread_detach(hiloEsperarDump);
    
    
}

void* manejarProcesoEsperandoDump(ProcesoEnEsperaDump* procesoEsperandoDump) {
    if (solicitar_dump_memoria(procesoEsperandoDump->proceso->PID)) {
        log_info(loggerKernel, "## (<%u>) - Memory Dump completado exitosamente", procesoEsperandoDump->proceso->PID);
    } else {
        log_error(loggerKernel, "## (<%u>) - Error en Memory Dump", procesoEsperandoDump->proceso->PID);
    }
    
    sem_wait(procesoEsperandoDump->semaforoDumpFinalizado);

    //procesoEsperandoDump->proceso->PID;
   
    char* PIDComoChar = pasarUnsignedAChar(procesoEsperandoDump->proceso->PID);
    sacarDeDiccionario(diccionarioProcesosEsperandoDump, PIDComoChar);
    cargarCronometro(procesoEsperandoDump->proceso,BLOCKED);
    log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoEsperandoDump->proceso->PID,"BLOCKED","READY");
    pasarAReady(procesoEsperandoDump->proceso,false);
    free(PIDComoChar);
    
    free(procesoEsperandoDump->semaforoDumpFinalizado);
    free(procesoEsperandoDump->semaforoMutex);
    free(procesoEsperandoDump);
    

    
    return NULL;
}

void syscall_IO(PCB* proceso, char* nombreIO, int64_t tiempo) {

    
    log_info(loggerKernel, "## (<%u>) - Solicitó syscall: IO", proceso->PID);

    

    sem_wait(semaforoMutexIO);
        

        DispositivoIO* dispositivo = NULL;
        dispositivo = leerDeDiccionario(diccionarioDispositivosIO,nombreIO);



        if (dispositivo == NULL) {
            log_error(loggerKernel, "## (<%u>) - Dispositivo IO %s no encontrado. Finalizando proceso", proceso->PID, nombreIO);
            pasarAExit(proceso,"EXECUTE");
            sem_post(semaforoMutexIO);
            return;
        }
        if (proceso == NULL) {
        log_error(loggerKernel, "## - No se encontró el PCB para syscall IO");
        exit(1);
        }


        

    sem_post(semaforoMutexIO);
    

    
    pasarABLoqueadoPorIO(proceso, tiempo, nombreIO);
    
}


PCB* buscarPCBEjecutando(uint32_t pid) {
    bool _mismoPID(void* arg) {
        NucleoCPU* nucleoEnEjecucion = (NucleoCPU*) arg;
        return nucleoEnEjecucion->procesoEnEjecucion->PID == pid;
    };

    bool _mismoPID2(void* arg) {
        PCB* procesoEnEjecucion = (PCB*) arg;
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

