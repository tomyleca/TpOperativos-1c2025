#include "monitoresListas.h"

sem_t* semaforoListaNew;
sem_t* semaforoListaReady;
sem_t* semaforoListaSwapReady;




void agregarALista(sem_t* semaforo,t_list* lista,void* elemento)
{
    sem_wait(semaforo);
    list_add(lista,elemento);
    sem_post(semaforo);
}

void* sacarDeLista(sem_t* semaforo,t_list* lista,unsigned int posicion)
{
    void* elem;
    sem_wait(semaforo);
    if(!list_is_empty(lista))
        elem = list_remove(lista,posicion);
    sem_post(semaforo);

    return elem;
}

void agregarAListaOrdenada(sem_t* semaforo,t_list* lista,void* elemento,bool (*funcionParaComparar) (void*,void*)){
    sem_wait(semaforo);
    list_add_sorted(lista,elemento,funcionParaComparar);
    sem_post(semaforo);
}


void ordenarLista(sem_t* semaforo,t_list* lista,bool (*funcionParaComparar) (void*,void*)){
    sem_wait(semaforo);
    list_sort(lista,funcionParaComparar);
    sem_post(semaforo);
}

void* sacarDeListaSegunCondicion(sem_t* semaforo,t_list* lista,bool (*condicion) (void*))
{
    void* elem;
    sem_wait(semaforo);
    if(!list_is_empty(lista))
        elem = list_remove_by_condition(lista,condicion);
    sem_post(semaforo);

    return elem;
}
