#include "monitoresDiccionarios.h"


sem_t* semaforoDiccionarioBlocked;
sem_t* semaforoDiccionarioIOBlocked;
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
    dictionary_get(diccionario,clave);
    sem_post(semaforo);
}