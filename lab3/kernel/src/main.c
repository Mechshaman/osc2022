#include "shell.h"
#include "smalloc.h"
#include "uart.h"

extern unsigned long dtb_addr;

void main(char* arg){
    
    dtb_addr = arg;

    shell_init();

    // char* string = simple_malloc(8);
    // string = "12345";
    // printf("%x\r\n",string);
    // char* string2 = simple_malloc(16);
    // string2 = "23456";
    // printf("%x\r\n",string2);
    // char* string3 = simple_malloc(8);
    // string3 = "67890";
    // printf("%x\r\n",string3);
    
    while(1) {
        shell_run();
    }
}