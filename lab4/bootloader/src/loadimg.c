#include "uart.h"

void loadimg(char* _dtb) {
    long long address = 0x80000;

    uart_puts("Send image via UART now!\n");

    // big endian
    int img_size = 0, i;
    for (i = 0; i < 4; i++) {
        img_size <<= 8;
        img_size |= (int)uart_getc_raw();
    }

    char *kernel = (char *)address;

    for (i = 0; i < img_size; i++) {
        char b = uart_getc_raw();
        printf("%d: %x\n",i,b);
        *(kernel + i) = b;
    }

    uart_puts("done\n");
    void (*start_os)(char*) = (void *)kernel;
    start_os(_dtb);
}