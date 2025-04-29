#include "monitoresDiccionarios.h"


sem_t* semaforoDiccionarioBlocked;
sem_t* semaforoDiccionarioProcesosBloqueados;
sem_t* semaforoDiccionarioBlockedSwap;


t_diccionarioConSemaforos* crearDiccionarioConSemaforos(){
    t_diccionarioConSemaforos* nuevoDiccionario = malloc(sizeof(t_diccionarioConSemaforos));
    nuevoDiccionario->diccionario = dictionary_create();
    nuevoDiccionario->semaforoMutex=malloc(sizeof(sem_t));
    nuevoDiccionario->semaforoCantElementos=malloc(sizeof(sem_t));
    sem_init(nuevoDiccionario->semaforoMutex,1,1);
    sem_init(nuevoDiccionario->semaforoCantElementos,1,0);
    return nuevoDiccionario;
}

void agregarADiccionario(t_diccionarioConSemaforos* diccionarioConSemaforos,char* clave, void* valor)
{
    sem_wait(diccionarioConSemaforos->semaforoMutex);
    dictionary_put(diccionarioConSemaforos->diccionario,clave,valor);
    sem_post(diccionarioConSemaforos->semaforoMutex);
}

void* leerDeDiccionario(t_diccionarioConSemaforos* diccionarioConSemaforos,char* clave)
{
    sem_wait(diccionarioConSemaforos->semaforoMutex);
    void* valor = dictionary_get(diccionarioConSemaforos->diccionario,clave);
    sem_post(diccionarioConSemaforos->semaforoMutex);

    return valor;
}

void* sacarDeDiccionario(t_diccionarioConSemaforos* diccionarioConSemaforos,char* clave)
{
    sem_wait(diccionarioConSemaforos->semaforoMutex);
    void* valor = dictionary_remove(diccionarioConSemaforos->diccionario,clave);
    sem_post(diccionarioConSemaforos->semaforoMutex);

    return valor;
}