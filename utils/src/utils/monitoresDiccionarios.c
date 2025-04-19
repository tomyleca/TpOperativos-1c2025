#include "monitoresDiccionarios.h"


sem_t* semaforoDiccionarioBlocked;
sem_t* semaforoDiccionarioProcesosBloqueados;
sem_t* semaforoDiccionarioBlockedSwap;


void agregarADiccionario(sem_t* semaforo,t_dictionary* diccionario,char* clave, void* valor)
{
    sem_wait(semaforo);
    dictionary_put(diccionario,"0",valor);
    sem_post(semaforo);
}

void* leerDeDiccionario(sem_t* semaforo,t_dictionary* diccionario,char* clave)
{
    sem_wait(semaforo);
    void* valor = dictionary_get(diccionario,clave);
    sem_post(semaforo);

    return valor;
}

void* sacarDeDiccionario(sem_t* semaforo,t_dictionary* diccionario,char* clave)
{
    sem_wait(semaforo);
    void* valor = dictionary_remove(diccionario,clave);
    sem_post(semaforo);

    return valor;
}