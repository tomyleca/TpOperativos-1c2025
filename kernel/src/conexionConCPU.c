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
    free(proceso->archivoPseudocodigo);

    //TODO avisar a memoria y loguear metricas
    inicializarProceso();
    free(proceso);
}




