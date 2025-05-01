#include "conexionConCPU.h"



void guardarDatosCPU(char* identificador,int fdConexion)
{
    nucleoCPU* nuevoNucleoCPU= malloc(sizeof(nuevoNucleoCPU));
    nuevoNucleoCPU->identificador= malloc(strlen(identificador));
    nuevoNucleoCPU->identificador=identificador;
    
    nuevoNucleoCPU->procesoEnEjecucion=NULL;
    nuevoNucleoCPU->fdConexion = fdConexion;

   
   

    agregarALista(listaCPUsLibres,nuevoNucleoCPU);
    sem_post(semaforoIntentarPlanificar);

    

}




void pasarAExit(PCB* proceso){
    log_info(loggerKernel,"## (<%u>) - Finaliza el proceso",proceso->PID);
    log_info(loggerKernel,"## (<%u>) Pasa del estado <%s> al estado <%s>",proceso->PID,"EXECUTE","EXIT");
    //TODO avisar a memoria 
    proceso->ME[EXIT]++;
    loggearMetricas(proceso);
    inicializarProceso();
    //TODO liberar cronometros
    free(proceso->archivoPseudocodigo);
    free(proceso);
}

void loggearMetricas(PCB* proceso)
{   
    log_info(loggerKernel,"## (<%u>) - Métricas de estado: NEW (%d) (%d), READY (%d) (%d) EXECUTE (%d) (%d) BLOCKED (%d) (%d) SWAP_BLOCKED (%d) (%d) SWAP_READY (%d) (%d) EXIT (%d) (%d)",
    proceso->PID,
    proceso->ME[0],proceso->MT[0],
    proceso->ME[1],proceso->MT[1],
    proceso->ME[2],proceso->MT[2],
    proceso->ME[3],proceso->MT[3],
    proceso->ME[4],proceso->MT[4],
    proceso->ME[5],proceso->MT[5],
    proceso->ME[6],proceso->MT[6]);
}



