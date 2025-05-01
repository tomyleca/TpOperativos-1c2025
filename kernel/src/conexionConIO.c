#include "kernel.h"
#include "../../utils/src/utils/conexionKernelIO.h"

void* esperarClientesIO(void* arg)
{
    while(1)
    {
        int* fdConexion = malloc(sizeof(int));
        *fdConexion = esperar_cliente(socket_kernel_io);
        log_info(loggerKernel, "Se conectó IO");
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
                
                nuevoDispositivoIO->semaforoDispositivoOcupado = malloc(sizeof(sem_t));
                sem_init(nuevoDispositivoIO->semaforoDispositivoOcupado,1,1);

                nuevoDispositivoIO->nombre=nombreIO;
                nuevoDispositivoIO->fdConexion= *fdConexion;
                agregarADiccionario(diccionarioDispositivosIO,nombreIO,nuevoDispositivoIO);
                break;
            
            case TERMINO_IO:
                uint32_t PID =recibir_uint32_t_del_buffer(buffer);
                char* nombre = recibir_string_del_buffer(buffer);
                manejarFinDeIO(PID,nombre);
                break;
            
            default:
                break;

                

        }

        

}

}

void avisarInicioIO(uint32_t PID,char* nombreIO,uint32_t tiempo)
{
    DispositivoIO* dispositivoIO = leerDeDiccionario(diccionarioDispositivosIO,nombreIO);
    
    if(dispositivoIO==NULL)
    {
        procesoEnEsperaIO* procesoAEliminar = sacarDeDiccionario(diccionarioProcesosBloqueados,pasarUnsignedAChar(PID));
        log_error(loggerKernel, "## (<%u>) Dispositivo IO no encontrado: %s.",procesoAEliminar->proceso->PID,nombreIO);
        pasarAExit(procesoAEliminar->proceso);
    }
    else 
    {
    sem_wait(dispositivoIO->semaforoDispositivoOcupado);

    t_paquete* paquete = crear_super_paquete(INICIA_IO_PROCESO);
    cargar_uint32_t_al_super_paquete(paquete,PID);
    cargar_uint32_t_al_super_paquete(paquete,tiempo);
    enviar_paquete(paquete,dispositivoIO->fdConexion);
    }


}

