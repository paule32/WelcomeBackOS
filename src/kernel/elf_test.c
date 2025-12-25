#include "kernel_api.h"

void main(void) {
    const char msg[] = "Hallo\n";
    sys_write(msg, sizeof(msg)-1);
    sys_exit(0);
}
