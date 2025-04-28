#include "kernel.h"
#include "../../utils/src/utils/conexionKernelIO.h"

void* esperarClientesIO(void* arg)
{
    while(1)
    {
        int* fdConexion = malloc(sizeof(int));
        *fdConexion = esperar_cliente(socket_kernel_io);
        log_info(logger_kernel, "Se conectó IO");
        pthread_t* nuevoHiloAtenderIO = malloc(sizeof(pthread_t));
        pthread_create(nuevoHiloAtenderIO,NULL,atenderIO,fdConexion);
    }
}


void* atenderIO(void* conexion)
{
    int* fdConexion = (int*)conexion;
    while(1)
    {
        opCodesKernelIO opCode = recibir_operacion(*fdConexion);
        t_buffer* buffer = recibiendo_super_paquete(*fdConexion); 

        switch(opCode)
        {
            case HANDSHAKE_IO_KERNEL:
                char* nombreIO = recibir_string_del_buffer(buffer);
                DispositivoIO* nuevoDispositivoIO = malloc(sizeof(DispositivoIO));
                nuevoDispositivoIO->nombre=nombreIO;
                nuevoDispositivoIO->ocupado=false;
                nuevoDispositivoIO->fdConexion= *fdConexion;
                agregarALista(listaDispositivosIO,nuevoDispositivoIO);
                break;
            case TERMINO_IO:
                uint32_t PID =recibir_uint32_t_del_buffer(buffer);
                //TODO
                break;
            default:
                break;

                

        }

        

}

}

void avisarInicioIO(uint32_t PID,char* nombreIO,uint32_t tiempo)
{
    DispositivoIO* dispositivoIO = buscarIOSegunNombre(nombreIO);
    
    if(dispositivoIO==NULL)
    {
        //TODO
    }
    else if(!dispositivoIO->ocupado)
    {
        t_paquete* paquete = crear_super_paquete(INICIA_IO_PROCESO);
        cargar_uint32_t_al_super_paquete(paquete,PID);
        cargar_uint32_t_al_super_paquete(paquete,tiempo);
        enviar_paquete(paquete,dispositivoIO->fdConexion);
    }
    else
    {
        //TODO 
    }

}

DispositivoIO* buscarIOSegunNombre(char* nombreIO)
{
    bool _mismoNombre(DispositivoIO* dispositivo)
    {
        return (strcmp(dispositivo->nombre,nombreIO) == 0);
    };

    return sacarDeListaSegunCondicion(listaDispositivosIO,_mismoNombre);
}
