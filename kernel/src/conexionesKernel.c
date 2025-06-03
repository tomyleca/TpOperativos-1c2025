#include "globalesKernel.h"

void iniciarConexiones()
{
    /****************CONEXION KERNEL CON MEMORIA*********************/

    //socket_kernel_memoria = crear_conexion(loggerKernel,ip_memoria,puerto_memoria);


}

void iniciarServidores()
{

    /****************CONEXION KERNEL CON IO*********************/

    //INICIO SERVIDOR KERNEL-IO
    socket_kernel_io = iniciar_servidor(loggerKernel, puerto_escucha_IO);
    log_info(loggerKernel, "## Servidor  iniciado para IO");
    
    
    
    
    

 


    /****************CONEXION KERNEL CON CPU*********************/

    //INICIO SERVIDOR KERNEL-CPU
    
    socket_kernel_cpu_dispatch = iniciar_servidor(loggerKernel, puerto_escucha_dispatch); 
    log_info(loggerKernel, "## Servidor DISPATCH iniciado");

    socket_kernel_cpu_interrupt = iniciar_servidor(loggerKernel, puerto_escucha_interrupt); 
    log_info(loggerKernel, "## Servidor INTERRUPT iniciado");
    
    

}

bool esperarOK2(int fdConexion)
{
    if(recibir_operacion(fdConexion)== OK)
        return 1;
    else
        return 0;

}