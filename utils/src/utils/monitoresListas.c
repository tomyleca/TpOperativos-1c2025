#include "monitoresListas.h"




t_listaConSemaforos* crearListaConSemaforos()
{
    t_listaConSemaforos* nuevaLista = malloc(sizeof(t_listaConSemaforos));
    nuevaLista->lista = list_create();
    nuevaLista->semaforoMutex=malloc(sizeof(sem_t));
    nuevaLista->semaforoCantElementos=malloc(sizeof(sem_t));
    sem_init(nuevaLista->semaforoMutex,1,1);
    sem_init(nuevaLista->semaforoCantElementos,1,0);
    return nuevaLista;
}


void agregarALista(t_listaConSemaforos* listaConSemaforos,void* elemento)
{
    sem_wait(listaConSemaforos->semaforoMutex);
        list_add(listaConSemaforos->lista,elemento);
    sem_post(listaConSemaforos->semaforoMutex);
    sem_post(listaConSemaforos->semaforoCantElementos);
}

void* sacarDeLista(t_listaConSemaforos* listaConSemaforos,unsigned int posicion)
{
    void* elem;
    
    sem_wait(listaConSemaforos->semaforoCantElementos);
    sem_wait(listaConSemaforos->semaforoMutex);
        elem = list_remove(listaConSemaforos->lista,posicion);
    sem_post(listaConSemaforos->semaforoMutex);
    

    return elem;
}

void agregarAListaOrdenada(t_listaConSemaforos* listaConSemaforos,void* elemento,bool (*funcionParaComparar) (void*,void*)){
    sem_wait(listaConSemaforos->semaforoMutex);
    list_add_sorted(listaConSemaforos->lista,elemento,funcionParaComparar);
    sem_post(listaConSemaforos->semaforoMutex);
    sem_post(listaConSemaforos->semaforoCantElementos);
}


void ordenarLista(t_listaConSemaforos* listaConSemaforos,bool (*funcionParaComparar) (void*,void*)){
    sem_wait(listaConSemaforos->semaforoMutex);
    list_sort(listaConSemaforos->lista,funcionParaComparar);
    sem_post(listaConSemaforos->semaforoMutex);
}

void* sacarDeListaSegunCondicion(t_listaConSemaforos* listaConSemaforos,bool (*condicion) (void*))
{
    void* elem;

    sem_wait(listaConSemaforos->semaforoCantElementos);
    sem_wait(listaConSemaforos->semaforoMutex);
        elem = list_remove_by_condition(listaConSemaforos->lista,condicion);
    sem_post(listaConSemaforos->semaforoMutex);

    return elem;
}
