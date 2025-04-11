#include "monitoresListas.h"

sem_t* semaforoListaNew;
sem_t* semaforoListaReady;


void iniciarSemaforosKernel()
{
    semaforoListaNew= malloc(sizeof(sem_t));
    semaforoListaReady = malloc(sizeof(sem_t));
  


   sem_init(semaforoListaNew,1,1);
   sem_init(semaforoListaReady,1,1);



}

void agregarALista(sem_t* semaforo,t_list* lista,void* elemento)
{
    sem_wait(semaforo);
    list_add(lista,elemento);
    sem_post(semaforo);
}

void* sacarDeLista(sem_t* semaforo,t_list* lista,unsigned int posicion)
{
    sem_wait(semaforo);
    void* elem = list_remove(lista,posicion);
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
