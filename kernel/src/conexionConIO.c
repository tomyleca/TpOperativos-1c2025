#include "kernel.h"
#include "../../utils/src/utils/conexionKernelIO.h"

void* esperarClientesIO(void* arg)
{
    while(1)
    {
        int* fdConexion = malloc(sizeof(int));
        *fdConexion = esperar_cliente(socket_kernel_io);
        log_debug(loggerKernel, "## Se conectó IO");
        pthread_t* nuevoHiloAtenderIO = malloc(sizeof(pthread_t));
        pthread_create(nuevoHiloAtenderIO,NULL,atenderInstanciaIO,fdConexion);
        pthread_detach(nuevoHiloAtenderIO);
    }
}


void* atenderInstanciaIO(void* conexion)
{
    int* fdConexion = (int*)conexion;
    InstanciaIO* instanciaIO = NULL;
    char* nombreIO;
    while(1)
    {
        int opCode = recibir_operacion(*fdConexion);
        t_buffer* buffer = recibiendo_super_paquete(*fdConexion); 
        


        switch(opCode)
        {
            case HANDSHAKE_IO_KERNEL:
                nombreIO = recibir_string_del_buffer(buffer);
                //sem_wait(semaforoMutexIO);
                if(leerDeDiccionario(diccionarioDispositivosIO,nombreIO) != NULL) // Si el IO ya existe, osea ya hay una instancia e este
                {
                    DispositivoIO* dispositivoIO= leerDeDiccionario(diccionarioDispositivosIO,nombreIO);
                    
                    InstanciaIO* instanciaIO = malloc(sizeof(InstanciaIO));
                    instanciaIO->estaLibre=true;
                    instanciaIO->fdConexion = *fdConexion;
                    instanciaIO->semaforoMutex = malloc(sizeof(sem_t));
                    sem_init(instanciaIO->semaforoMutex,1,1);
                    agregarALista(dispositivoIO->listaInstancias,instanciaIO);
                }

                else{ // Es la primera instancia de este dispositivo 
                    DispositivoIO* nuevoDispositivoIO = malloc(sizeof(DispositivoIO));
                    nuevoDispositivoIO->nombre=nombreIO;
                    nuevoDispositivoIO->listaInstancias = crearListaConSemaforos();
                    nuevoDispositivoIO->colaEsperandoIO = crearListaConSemaforos();
                    
                    InstanciaIO* instanciaIO = malloc(sizeof(InstanciaIO));
                    instanciaIO->estaLibre=true;
                    instanciaIO->fdConexion = *fdConexion;
                    instanciaIO->semaforoMutex = malloc(sizeof(sem_t));
                    sem_init(instanciaIO->semaforoMutex,1,1);
                    
                    agregarALista(nuevoDispositivoIO->listaInstancias,instanciaIO);
                    
                    agregarADiccionario(diccionarioDispositivosIO,nombreIO,nuevoDispositivoIO);
                }
                //sem_post(semaforoMutexIO);
                break;
            
            case TERMINO_IO:
                uint32_t PID =recibir_uint32_t_del_buffer(buffer);
                char* nombre = recibir_string_del_buffer(buffer);
                manejarFinDeIO(PID,nombre,*fdConexion);
                free(nombre);

                break;
            
            //case 0:
            case -1:
                manejarDesconexionDeIO(nombreIO,*fdConexion);
                //shutdown(*fdConexion, SHUT_RDWR);
                close(*fdConexion);
                pthread_exit(NULL);
                break;
            
            default:
                break;

                

        }
    limpiarBuffer(buffer);
        

}

}

int avisarInicioIO(ProcesoEnEsperaIO* procesoEnEsperaIO,char* nombreIO,int64_t tiempo)
{
    
        InstanciaIO* instanciaIO = NULL;
        DispositivoIO* dispositivoIO = leerDeDiccionario(diccionarioDispositivosIO,nombreIO);
        if(dispositivoIO == NULL)
            {
            sem_post(semaforoMutexIO);
            return -1;
            }
    
        instanciaIO = leerDeListaSegunCondicion(dispositivoIO->listaInstancias,instanciaLibre); //Busco la primer instancia libre
        

        if(instanciaIO == NULL) // No hay instancias libres
        {
            agregarALista(dispositivoIO->colaEsperandoIO,procesoEnEsperaIO); // Si el dispositivo ya esta ocupado entra acá
            log_debug(loggerKernel,"## (<%u>) Dispositivo IO: %s ocupado. Se agrega el PID a la Cola de Espera del dispositivo",procesoEnEsperaIO->proceso->PID,dispositivoIO->nombre);
            
        }
        else
        {
        sem_wait(instanciaIO->semaforoMutex),
            instanciaIO->PIDEnIO= procesoEnEsperaIO->proceso->PID;
            instanciaIO->estaLibre=false;
        sem_post(instanciaIO->semaforoMutex);

        t_paquete* paquete = crear_super_paquete(INICIA_IO_PROCESO);
        cargar_uint32_t_al_super_paquete(paquete,procesoEnEsperaIO->proceso->PID);
        cargar_int64_t_al_super_paquete(paquete,tiempo);
        enviar_paquete(paquete,instanciaIO->fdConexion);
        eliminar_paquete(paquete);
        }
    
    
    return 0;


}




bool instanciaLibre(InstanciaIO* instanciaIO)
{
    bool estaLibre;

    sem_wait(instanciaIO->semaforoMutex);
        estaLibre = instanciaIO->estaLibre;
    sem_post(instanciaIO->semaforoMutex);

    return estaLibre;

}

void manejarDesconexionDeIO(char* nombreDispositivoIO, int fdConexion)
{
    sem_wait(semaforoMutexIO);
        log_debug(loggerKernel,"# Se desconectó IO: %s",nombreDispositivoIO);
        bool _esInstancia(InstanciaIO* instanciaIO)
        {
            return instanciaIO->fdConexion == fdConexion;  //Busco la instancia por conexixón, que es lo que las diferencia
        };
        
        DispositivoIO* dispositivoIO = leerDeDiccionario(diccionarioDispositivosIO,nombreDispositivoIO);
        InstanciaIO* instanciaIO = leerDeListaSegunCondicion(dispositivoIO->listaInstancias,_esInstancia);
        
        char* clave=pasarUnsignedAChar(instanciaIO->PIDEnIO);
        ProcesoEnEsperaIO* procesoEnEsperaIO = leerDeDiccionario(diccionarioProcesosBloqueados,clave);
        exitDeProcesoBLoqueadoPorIO(procesoEnEsperaIO);
        free(clave);

        sacarElementoDeLista(dispositivoIO->listaInstancias,instanciaIO);

        if(chequearListaVacia(dispositivoIO->listaInstancias)) //Si despues de sacar la instancia no quedan más paso a exit todos los proceso esperando el dispositivo
        {
            if(!chequearListaVacia(dispositivoIO->colaEsperandoIO)) //Chequeo que no este vacia tampoco
                {   
                    log_debug(loggerKernel,"# Finalizando procesos encolados en dispositivo: %s",nombreDispositivoIO);
                    list_iterate(dispositivoIO->colaEsperandoIO->lista,exitDeProcesoBLoqueadoPorIO);
                }
        }
        borrarListaConSemaforos(dispositivoIO->listaInstancias);
        borrarListaConSemaforos(dispositivoIO->colaEsperandoIO);
        sacarDeDiccionario(diccionarioDispositivosIO,nombreDispositivoIO);
        free(dispositivoIO->nombre);
        free(dispositivoIO);
    
    sem_post(semaforoMutexIO);
}


void exitDeProcesoBLoqueadoPorIO(ProcesoEnEsperaIO* procesoEnEsperaIO)
{
    
    
    if(procesoEnEsperaIO != NULL)
    {
    sem_wait(procesoEnEsperaIO->semaforoMutex); //Para no interrumpir manejarProcesoBloqueado o hiloContadorSwap a la mitad 
        esperarCancelacionDeHilo(procesoEnEsperaIO->hiloContadorSwap); //Cancelo el hilo contadorSwap, para que no tire seg fault cuando haga free del semaforoMutex
        esperarCancelacionDeHilo(procesoEnEsperaIO->hiloManejoBloqueado); //Cancelo este hilo que esta esperando el fin de IO
        char* clave= pasarUnsignedAChar(procesoEnEsperaIO->proceso->PID);
        sacarDeDiccionario(diccionarioProcesosBloqueados,clave);
        free(clave);
        if(procesoEnEsperaIO->estaENSwap)
            pasarAExit(procesoEnEsperaIO->proceso,"SWAP_BLOCKED");
        else
            pasarAExit(procesoEnEsperaIO->proceso,"BLOCKED");
       
        free(procesoEnEsperaIO->semaforoMutex);
        free(procesoEnEsperaIO->semaforoIOFinalizada);
        free(procesoEnEsperaIO);
     
    }
    
        
       
}
