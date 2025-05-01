#include <semaphore.h>
#include <commons/collections/list.h>
#include <stdlib.h>


typedef struct{
    t_list* lista;
    sem_t* semaforoMutex;
    sem_t* semaforoCantElementos;
} t_listaConSemaforos;


/**
*@brief Crea una lista con semaforos(mutex para asegurar mutua exclusion y seamforoCantElementos para asegurarse que no este vacía)
*/
extern t_listaConSemaforos* crearListaConSemaforos();


/**
*@brief Agrega el elemento a la lista. Utiliza semáforos para evitar condiciones de carrera.
*/
extern void agregarALista(t_listaConSemaforos* listaConSemaforos,void* elemento);

/**
*@brief Saca el elemento que se encuentra en la posición dada de la lista. Utiliza semáforos para evitar condiciones de carrera.
*/
extern void* sacarDeLista(t_listaConSemaforos* listaConSemaforos,unsigned int posicion);


/**
*@brief Agrega el elemento a una lista según un orden determinado(que se pasa como parámetro). Utiliza semáforos para evitar condiciones de carrera.
*/
extern void agregarAListaOrdenada(t_listaConSemaforos* listaConSemaforos,void* elemento,bool (*funcionParaComparar) (void*,void*));

/**
*@brief Ordena una lista según un orden determinado(que se pasa como parámetro). Utiliza semáforos para evitar condiciones de carrera.
*/
extern void ordenarLista(t_listaConSemaforos* listaConSemaforos,bool (*funcionParaComparar) (void*,void*));

/**
*@brief Saca el primer elemento de una lista que coincida con la condicion dada. Utiliza semáforos para evitar condiciones de carrera.
*/
extern void* sacarDeListaSegunCondicion(t_listaConSemaforos* listaConSemaforos,bool (*condicion) (void*));


/**
*@brief Devuelve el elemento que se encuentra en la posición dada de la lista. Utiliza semáforos para evitar condiciones de carrera.
*/
extern void* leerDeLista(t_listaConSemaforos* listaConSemaforos,unsigned int posicion);

/**
*@brief Devuelve el primer elemento de una lista que coincida con la condicion dada. Utiliza semáforos para evitar condiciones de carrera.
*/
void* leerDeListaSegunCondicion(t_listaConSemaforos* listaConSemaforos,bool (*condicion) (void*));

/**
*@brief Chequea si la lista esta vacia. Utiliza semáforos para evitar condiciones de carrera.
*/
extern bool chequearListaVacia(t_listaConSemaforos* listaConSemaforos);


/**
*@brief Saca un elemento dado de la lista. Retorna true si lo encontró, false en otro caso. Utiliza semáforos para evitar condiciones de carrera.
*/
extern bool sacarElementoDeLista(t_listaConSemaforos* listaConSemaforos,void* elem);


/**
*@brief Hace clean de una lista con semaforos y borra sus semaforos, sin borrar los elementos referenciados en la lista. 
*/
extern void borrarListaConSemaforos(t_listaConSemaforos* listaConSemaforos);