#include <kernel.h>



int main(int argc, char* argv[]) {
    
    //saludar("kernel");
    t_config* config_kernel = iniciar_config("kernel.config");
    leer_config_kernel(config_kernel);
    return 0;
}

void leer_config_kernel(t_config* config_kernel) {
    
    ip_memoria = config_get_string_value(config_kernel, "IP_MEMORIA");
    puerto_memoria = config_get_int_value(config_kernel, "PUERTO_MEMORIA");
    puerto_escucha_dispatch = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_DISPATCH");
    puerto_escucha_interrupt = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_INTERRUPT");
    puerto_escucha_IO = config_get_int_value(config_kernel, "PUERTO_ESCUCHA_IO");
    algoritmo_planificacion = config_get_string_value(config_kernel, "ALGORITMO_PLANIFICACION");
    tiempo_suspension = config_get_int_value(config_kernel, "TIEMPO_SUSPENSION");
    log_level = config_get_string_value(config_kernel, "LOG_LEVEL");

}
