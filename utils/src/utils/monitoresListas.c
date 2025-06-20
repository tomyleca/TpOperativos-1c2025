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


void agregarAListaSinRepetidos(t_listaConSemaforos* listaConSemaforos,void* nuevoElemento)
{
    bool _esIgualANuevoElemento(void* elementoEnLista)
    {
        return elementoEnLista == nuevoElemento;
    };

    sem_wait(listaConSemaforos->semaforoMutex);
    
    if(list_find(listaConSemaforos->lista,_esIgualANuevoElemento) == NULL) //Si ningún elemento es igual al elemento a agregar, lo agrego
        {
        list_add(listaConSemaforos->lista,nuevoElemento);
        sem_post(listaConSemaforos->semaforoCantElementos);
        }

    sem_post(listaConSemaforos->semaforoMutex);
    
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

    
    sem_wait(listaConSemaforos->semaforoMutex);
        elem = list_remove_by_condition(listaConSemaforos->lista,condicion);
    sem_post(listaConSemaforos->semaforoMutex);

    return elem;
}

void* leerDeLista(t_listaConSemaforos* listaConSemaforos,unsigned int posicion)
{
    void* elem;
    
    sem_wait(listaConSemaforos->semaforoCantElementos);
    sem_wait(listaConSemaforos->semaforoMutex);
        elem = list_get(listaConSemaforos->lista,posicion);
    sem_post(listaConSemaforos->semaforoMutex);
    sem_post(listaConSemaforos->semaforoCantElementos);
    
    return elem;
}

bool chequearListaVacia(t_listaConSemaforos* listaConSemaforos)
{
    bool vacia = false;
    t_list* lista = listaConSemaforos->lista;
    
    sem_wait(listaConSemaforos->semaforoMutex);
        vacia = list_is_empty(lista);
    sem_post(listaConSemaforos->semaforoMutex);
    
    return vacia;
}

bool sacarElementoDeLista(t_listaConSemaforos* listaConSemaforos,void* elem)
{
    bool elementoEncontrado;


    sem_wait(listaConSemaforos->semaforoMutex);
        elementoEncontrado = list_remove_element(listaConSemaforos->lista,elem);
    sem_post(listaConSemaforos->semaforoMutex);



    return elementoEncontrado;
}

void* leerDeListaSegunCondicion(t_listaConSemaforos* listaConSemaforos,bool (*condicion) (void*))
{
    void* elem;
    
    sem_wait(listaConSemaforos->semaforoMutex);
        elem = list_find(listaConSemaforos->lista,condicion);
    sem_post(listaConSemaforos->semaforoMutex);

    return elem;
}

void borrarListaConSemaforos(t_listaConSemaforos* listaConSemaforos)
{
    list_destroy(listaConSemaforos->lista);
    free(listaConSemaforos->semaforoCantElementos);
    free(listaConSemaforos->semaforoMutex);
    free(listaConSemaforos);
}

