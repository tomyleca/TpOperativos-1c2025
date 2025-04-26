#include <semaphore.h>
#include <commons/collections/dictionary.h>
#include <stdlib.h>

typedef struct{
    t_dictionary* diccionario;
    sem_t* semaforoMutex;
    sem_t* semaforoCantElementos;
} t_diccionarioConSemaforos;




t_diccionarioConSemaforos* crearDiccionarioConSemaforos(); 
void agregarADiccionario(t_diccionarioConSemaforos* diccionarioConSemaforos,char* clave, void* valor);
void* leerDeDiccionario(t_diccionarioConSemaforos* dicionarioConSemaforos,char* clave);
void* sacarDeDiccionario(t_diccionarioConSemaforos* diccionarioConSemaforos,char* clave);




