#include "kernel.h"

void iniciarConexiones()
{
    /****************CONEXION KERNEL CON MEMORIA*********************/

    socket_kernel_memoria = crear_conexion(loggerKernel,ip_memoria,puerto_memoria);

    /****************CONEXION KERNEL CON IO*********************/

    //INICIO SERVIDOR KERNEL-IO
    /*socket_kernel_io = iniciar_servidor(loggerKernel, puerto_escucha_IO);
    log_info(loggerKernel, "Servidor  iniciado para IO");*/
    
    
    
    
    

 


    /****************CONEXION KERNEL CON CPU*********************/

    //INICIO SERVIDOR KERNEL-CPU

    socket_kernel_cpu_dispatch = iniciar_servidor(loggerKernel, puerto_escucha_dispatch); 
    log_info(loggerKernel, "Servidor DISPATCH iniciado");

    socket_kernel_cpu_interrupt = iniciar_servidor(loggerKernel, puerto_escucha_interrupt); 
    log_info(loggerKernel, "Servidor INTERRUPT iniciado");
    
    //cliente_kernel_dispatch = esperar_cliente(socket_kernel_cpu_dispatch);
    
    //cliente_kernel_interrupt = esperar_cliente(socket_kernel_cpu_interrupt);



}


void cerrarConexiones()
{
    log_info(loggerKernel, "Finalizando conexión");
    liberar_conexion(socket_kernel_cpu_dispatch);
    liberar_conexion(socket_kernel_cpu_interrupt);
    liberar_conexion(socket_kernel_io);
    liberar_conexion(socket_kernel_memoria);
}