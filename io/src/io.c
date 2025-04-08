#include <io.h>

int main(int argc, char* argv[]) {
    saludar("io");

    config_io = crear_config();
    socket_io_kernel=crear_conexion();

    return 0;
}
