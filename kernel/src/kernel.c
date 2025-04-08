#include <kernel.h>

int main(int argc, char* argv[]) {
    
    //saludar("kernel");
    archivo_config= "../kernel/kernel.config";
    leer_archivo_config(archivo_config);
    return 0;
}
