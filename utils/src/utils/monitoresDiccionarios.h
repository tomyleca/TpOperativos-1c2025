#include <semaphore.h>
#include <commons/collections/dictionary.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct{
    t_dictionary* diccionario;
    sem_t* semaforoMutex;
    sem_t* semaforoCantElementos;
} t_diccionarioConSemaforos;




t_diccionarioConSemaforos* crearDiccionarioConSemaforos(); 

extern void agregarADiccionario(t_diccionarioConSemaforos* diccionarioConSemaforos,char* clave, void* valor);
extern void* leerDeDiccionario(t_diccionarioConSemaforos* dicionarioConSemaforos,char* clave);
extern void* sacarDeDiccionario(t_diccionarioConSemaforos* diccionarioConSemaforos,char* clave);
extern void* destruirDiccionario(t_diccionarioConSemaforos* diccionario,void(*element_destroyer)(void*));



/**
 * @brief Pasa un valor uint32_t a un char.
*/
extern char* pasarUnsignedAChar(uint32_t unsigned_);




