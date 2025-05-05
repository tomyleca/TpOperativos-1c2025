#include <io.h>

int main(int argc, char* argv[]) {
    

    //INICIO Y LEO CONFIG
    config_io = iniciar_config("io.config");
    leerConfigIO(config_io);
    
    //INICIO LOGGER
    loggerIO = iniciar_logger("ioLogger.log","ioLogger",log_level);

 

        if(argc!=2)
    {
        //TODO logger error
        return EXIT_FAILURE;
    }


    conectarseAKernel(argv[1]);
    
    while(1)
    {
    uint32_t PID = recibirProcesoEnIOEIniciarUsleep();
    avisarFinDeIO(PID,argv[1]);
    }



    



 


    return 0;
}

void leerConfigIO(t_config* config_io) 
{
    ip_kernel = config_get_string_value(config_io, "IP_KERNEL");
    puerto_kernel = config_get_string_value(config_io, "PUERTO_KERNEL");
    log_level = log_level_from_string(config_get_string_value(config_io, "LOG_LEVEL"));
    

}

void conectarseAKernel(char* nombre)
{
    conexionKernel = crear_conexion(loggerIO,ip_kernel,puerto_kernel);
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion=HANDSHAKE_IO_KERNEL;
    cargar_string_al_super_paquete(paquete,nombre);
    enviar_paquete(paquete,conexionKernel);
    
    
}

uint32_t recibirProcesoEnIOEIniciarUsleep()
{
    opCodesKernelIO opCode = recibir_operacion(conexionKernel);
    uint32_t PID;
    if(opCode==INICIA_IO_PROCESO)
    {    
        t_buffer* buffer = recibiendo_super_paquete(conexionKernel); 
        PID = recibir_uint32_t_del_buffer(buffer);
        uint32_t tiempo = recibir_uint32_t_del_buffer(buffer);

        log_info(loggerIO,"## PID: <%u> - Inicio de IO - Tiempo: <%u>",PID,tiempo);
        
        usleep(tiempo);
    }

    return PID;
}

void avisarFinDeIO(uint32_t PID,char* nombreIO)
{
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion=TERMINO_IO;
    cargar_uint32_t_al_super_paquete(paquete,PID);
    cargar_string_al_super_paquete(paquete,nombreIO);
    enviar_paquete(paquete,conexionKernel);


    log_info(loggerIO,"## PID: <%u> - Fin de IO",PID);
    

}