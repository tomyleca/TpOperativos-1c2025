#include "kernel.h"
#include "../../utils/src/utils/opCodesKernelIO.h"

void* atenderIO(void* arg)
{
    while(1)
    {
        int fdConexion = esperar_cliente(socket_kernel_io);
        log_info(loggerKernel, "Se conectó IO");
        
        
        opCodesKernelIO opCode = recibir_operacion(fdConexion);
        t_buffer* buffer = recibiendo_super_paquete(fdConexion); 

        switch(opCode)
        {
            case HANDSHAKE_IO_KERNEL:
                char* nombreIO = recibir_string_del_buffer(buffer);
                DispositivoIO* nuevoDispositivoIO = malloc(sizeof(DispositivoIO));
                nuevoDispositivoIO->nombre=nombreIO;
                nuevoDispositivoIO->ocupado=false;
                agregarALista(semaforoListaDispositivosIO,listaDispositivosIO,nuevoDispositivoIO);
                break;
                

        }

        

}

}