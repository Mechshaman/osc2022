#include "uart.h"
#include "relocate.h"
#include "loadimg.h"

int rel = 1;
char* _dtb;

void main(char* arg){

    if(rel){
        _dtb = arg;
        rel = 0;
        relocate();
    }
    
    uart_init();

    uart_puts("Waiting for loading kernel...\r\n");

    loadimg(_dtb);

}