#include "kernel.h"
#include "../../utils/src/utils/conexionKernelIO.h"

void* esperarClientesIO(void* arg)
{
    while(1)
    {
        int* fdConexion = malloc(sizeof(int));
        *fdConexion = esperar_cliente(socket_kernel_io);
        log_info(loggerKernel, "## Se conectó IO");
        pthread_t* nuevoHiloAtenderIO = malloc(sizeof(pthread_t));
        pthread_create(nuevoHiloAtenderIO,NULL,atenderInstanciaIO,fdConexion);
    }
}


void* atenderInstanciaIO(void* conexion)
{
    int* fdConexion = (int*)conexion;
    InstanciaIO* instanciaIO = NULL;
    while(1)
    {
        opCodesKernelIO opCode = recibir_operacion(*fdConexion);
        t_buffer* buffer = recibiendo_super_paquete(*fdConexion); 
        


        switch(opCode)
        {
            case HANDSHAKE_IO_KERNEL:
                char* nombreIO = recibir_string_del_buffer(buffer);
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
                break;
            
            case TERMINO_IO:
                uint32_t PID =recibir_uint32_t_del_buffer(buffer);
                char* nombre = recibir_string_del_buffer(buffer);
                manejarFinDeIO(PID,nombre,*fdConexion);
                break;
            
            //case 0:
            case -1:
                log_info(loggerKernel,"# Se desconectó IO: ");
                pthread_exit(NULL);
                break;
            
            default:
                break;

                

        }

        

}

}

void avisarInicioIO(procesoEnEsperaIO* procesoEnEsperaIO,char* nombreIO,int64_t tiempo)
{
    DispositivoIO* dispositivoIO = leerDeDiccionario(diccionarioDispositivosIO,nombreIO);
    InstanciaIO* instanciaIO = leerDeListaSegunCondicion(dispositivoIO->listaInstancias,instanciaLibre);
    
    if(instanciaIO == NULL) // No hay instancias libres
    {
        agregarALista(dispositivoIO->colaEsperandoIO,procesoEnEsperaIO); // Si el dispositivo ya esta ocupado entra acá
        log_info(loggerKernel,"## <%u> Dispositivo IO: %s ocupado. Se agrega el PID a la Cola de Espera del dispositivo",procesoEnEsperaIO->proceso->PID,dispositivoIO->nombre);
        
    }
    else
    {
    sem_wait(instanciaIO->semaforoMutex),
        instanciaIO->estaLibre=false;
    sem_post(instanciaIO->semaforoMutex);

    t_paquete* paquete = crear_super_paquete(INICIA_IO_PROCESO);
    cargar_uint32_t_al_super_paquete(paquete,procesoEnEsperaIO->proceso->PID);
    cargar_uint32_t_al_super_paquete(paquete,tiempo);
    enviar_paquete(paquete,instanciaIO->fdConexion);
    free(paquete);
    }

    


}




bool instanciaLibre(InstanciaIO* instanciaIO)
{
    bool estaLibre;

    sem_wait(instanciaIO->semaforoMutex);
        estaLibre = instanciaIO->estaLibre;
    sem_post(instanciaIO->semaforoMutex);

    return estaLibre;

}