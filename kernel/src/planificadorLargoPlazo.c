#include "kernel.h"

void *inicializarProceso(){
    PCB* procesoAInicializar;
    sem_wait(semaforoAprietaEnter);
    while(1)

    {
        sem_wait(semaforoInicializarProceso);

            if (!list_is_empty(listaProcesosSwapReady->lista)) //Esto es para darle mas prioridad a la lista Swap Ready
            {
                procesoAInicializar= leerDeLista(listaProcesosSwapReady,0);
                
                if(des_suspender_proceso_memoria(procesoAInicializar->PID))
                {   
                    if(sacarElementoDeLista(listaProcesosSwapReady,procesoAInicializar)== false)
                    {
                        log_debug(loggerKernel,"ERROR AL INICIALIZAR PROCESO");
                        
                    }
                    log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAInicializar->PID,"SWAP_READY","READY");
                    cargarCronometro(procesoAInicializar,SWAP_READY);
                    pasarAReady(procesoAInicializar,false);
                    sem_post(semaforoInicializarProceso); // Mientras la respuesta sea OK sigo intentando inicializar procesos
                    
                
                }
        
            }
            else if (!list_is_empty(listaProcesosNew->lista))
            {
                procesoAInicializar = leerDeLista(listaProcesosNew,0);
                

                if(mandarDatosProcesoAMemoria(procesoAInicializar) == OK)
                {   
                    if(sacarElementoDeLista(listaProcesosNew,procesoAInicializar)== false)
                    {
                        log_debug(loggerKernel,"ERROR AL INICIALIZAR PROCESO");
                        
                    }
                    log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",procesoAInicializar->PID,"NEW","READY");
                    cargarCronometro(procesoAInicializar,NEW);
                    pasarAReady(procesoAInicializar,false);
                    sem_post(semaforoInicializarProceso); // Mientras la respuesta sea OK sigo intentando inicializar procesos
                }
                

            }


    
    }
    return 0;
}

int mandarDatosProcesoAMemoria(PCB* proceso)
{
    int socketKernelMemoria = crear_conexion(loggerKernel,ip_memoria,puerto_memoria);
    t_paquete* paquete = crear_super_paquete(GUARDAR_PROCESO_EN_MEMORIA);
    cargar_uint32_t_al_super_paquete(paquete,proceso->PID);
    cargar_uint32_t_al_super_paquete(paquete,proceso->tam);
    cargar_string_al_super_paquete(paquete,proceso->archivoPseudocodigo);
    enviar_paquete(paquete,socketKernelMemoria);
    int respuesta = recibir_operacion(socketKernelMemoria);
    
    
    if(respuesta == NO_HAY_MEMORIA)
    {
        log_error(loggerKernel,"## (<%u>) No hay suficiente memoria para alojar el proceso",proceso->PID);
    }

    eliminar_paquete(paquete);
    //shutdown(socketKernelMemoria, SHUT_RDWR);
    close(socketKernelMemoria);

    return respuesta;
    
}



bool menorTam(void* arg1, void* arg2)
{
    PCB* PCB1 = (PCB*) arg1;
    PCB* PCB2 = (PCB*) arg2;
    return PCB1->tam <= PCB2->tam;
}



void pasarAReady(PCB* proceso, bool desalojado){
    char* PIDComoChar = pasarUnsignedAChar(proceso->PID);
    if(leerDeDiccionario(diccionarioProcesosBloqueados,PIDComoChar)!= NULL || leerDeDiccionario(diccionarioProcesosEsperandoDump,PIDComoChar)!= NULL) //PARA SRT
    {
        free(PIDComoChar);
        return;
    }
    free(PIDComoChar);
    agregarAListaSinRepetidos(listaProcesosReady,proceso);
    temporal_resume(proceso->cronometros[READY]);
    proceso->ME[READY]++;
    
    if(algoritmoDePlanificacionInt == SRT && desalojado == false)
    {
        chequearSiHayDesalojo(proceso->estimadoSiguienteRafaga);
    }

    
}
