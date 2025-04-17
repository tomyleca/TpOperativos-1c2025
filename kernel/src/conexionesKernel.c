#include "kernel.h"

void iniciarConexiones()
{
    /****************CONEXION KERNEL CON MEMORIA*********************/

    socket_kernel_memoria = crear_conexion(logger_kernel,ip_memoria,puerto_memoria);

    /****************CONEXION KERNEL CON IO*********************/

    //INICIO SERVIDOR KERNEL-IO
    socket_kernel_io = iniciar_servidor(logger_kernel, puerto_escucha_IO);
    log_info(logger_kernel, "Servidor  iniciado para IO");
    
    cliente_kernel = esperar_cliente(socket_kernel_io);
    log_info(logger_kernel, "Se conectó IO");

    //CIERRO
    log_info(logger_kernel, "Finalizando conexión");
    liberar_conexion(socket_kernel_io);


    /****************CONEXION KERNEL CON CPU*********************/

    //INICIO SERVIDOR KERNEL-CPU

    socket_kernel_cpu_dispatch = iniciar_servidor(logger_kernel, puerto_escucha_dispatch); 
    log_info(logger_kernel, "Servidor DISPATCH iniciado");

    socket_kernel_cpu_interrupt = iniciar_servidor(logger_kernel, puerto_escucha_interrupt); 
    log_info(logger_kernel, "Servidor INTERRUPT iniciado");
    
    cliente_kernel_dispatch = esperar_cliente(socket_kernel_cpu_dispatch);
    
    cliente_kernel_interrupt = esperar_cliente(socket_kernel_cpu_interrupt);



}


void cerrarConexiones()
{
    log_info(logger_kernel, "Finalizando conexión");
    liberar_conexion(socket_kernel_cpu_dispatch);
    liberar_conexion(socket_kernel_cpu_interrupt);
    liberar_conexion(socket_kernel_io);
    liberar_conexion(socket_kernel_memoria);
}