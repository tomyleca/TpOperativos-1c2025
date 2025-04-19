#include <semaphore.h>
#include <commons/collections/dictionary.h>
#include <stdlib.h>

void agregarADiccionario(sem_t* semaforo,t_dictionary* diccionario,char* clave, void* valor);
void* leerDeDiccionario(sem_t* semaforo,t_dictionary* dicionario,char* clave);
void* sacarDeDiccionario(sem_t* semaforo,t_dictionary* diccionario,char* clave);




