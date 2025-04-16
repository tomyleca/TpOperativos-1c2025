#include "conexion-cpu-memoria.h"

/*void atender_memoria()
{ 
    t_buffer* buffer;
    int cod_op;
    int direccion_fisica;

    while(1) 
    {
        cod_op = recibir_operacion(socket_cpu_memoria);
        switch (cod_op) 
        {
            case CPU_RECIBE_INSTRUCCION_MEMORIA: 
                //ACA LLEGA LA SOLICITUD DE LA INSTRUCCION DE MEMORIA
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                contexto->pid = recibir_string_del_buffer(buffer);
                instruccion_recibida = recibir_string_del_buffer(buffer); // instruccion_recibida se usa en instruccion.c
                free(buffer);
                break;

            case CPU_RECIBE_CONTEXTO:  
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                contexto->pid = recibir_int_del_buffer(buffer);
                cargar_registros(buffer);
                fetch(socket_cpu_memoria);
                //recibir_contexto(); 
                free(buffer);
                break;

            case CPU_RECIBE_OK_DE_LECTURA:
               buffer = recibiendo_super_paquete(socket_cpu_memoria);
                contexto->pid = recibir_int_del_buffer(buffer);
                direccion_fisica = recibir_int_del_buffer(buffer); // POR EL MOMENTO TRATAMOS A LA DIRECCION FISICA COMO INT 
                valor_leido_de_memoria_32 = recibir_int_del_buffer(buffer);
                free(buffer);
                break;

            case CPU_RECIBE_OK_DE_ESCRITURA:
                buffer = recibiendo_super_paquete(socket_cpu_memoria);
                contexto->pid = recibir_int_del_buffer(buffer);
                direccion_fisica = recibir_int_del_buffer(buffer);                
                free(buffer); 
                break;

            case CPU_RECIBE_OK_ACTUALIZAR_CONTEXTO: //MANDO PID CON LOS REGISTROS O PCB COMPLETO
               buffer = recibiendo_super_paquete(socket_cpu_memoria);
                contexto->pid = recibir_int_del_buffer(buffer);
                if(algoritmo == 1)
                {
                    mandar_mensaje_kernel_cpu_ejecuta_quantum();
                } 
                printf("Antes de semaforo de contexto\n");  
                free(buffer); 
                break;
            case -1:
                log_error(logger_cpu, "MEMORIA se desconecto. Terminando servidor");
                exit(1);
            default:
                log_warning(logger_cpu, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}*/




