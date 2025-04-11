#include <semaphore.h>
#include <commons/collections/list.h>
#include <stdlib.h>



extern sem_t* semaforoListaNew;
extern sem_t* semaforoListaReady;

extern void iniciarSemaforosKernel();

/**
*@brief Agrega el elemento a la lista. Utiliza semáforos para evitar condiciones de carrera.
*/
extern void agregarALista(sem_t* semaforo,t_list* lista,void* elemento);

/**
*@brief Saca el elemento que se encuentra en la posición dada de la lista. Utiliza semáforos para evitar condiciones de carrera.
*/
extern void* sacarDeLista(sem_t* semaforo,t_list* lista,unsigned int posicion);


/**
*@brief Agrega el elemento a una lista según un orden determinado(que se pasa como parámetro). Utiliza semáforos para evitar condiciones de carrera.
*/
extern void agregarAListaOrdenada(sem_t* semaforo,t_list* lista,void* elemento,bool (*funcionParaComparar) (void*,void*));

/**
*@brief Ordena una lista según un orden determinado(que se pasa como parámetro). Utiliza semáforos para evitar condiciones de carrera.
*/
extern void ordenarLista(sem_t* semaforo,t_list* lista,bool (*funcionParaComparar) (void*,void*));