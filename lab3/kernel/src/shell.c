#include "shell.h"
#include "mailbox.h"
#include "string.h"
#include "reboot.h"
#include "uart.h"
#include "cpio.h"
#include "dtb.h"
#include "timer.h"
#include "exception.h"
#include "task.h"

#define CMD_LEN 128

void init_cpio_addr()
{
    fdt_traverse(initramfs_callback);
}

void shell_init(){
    init_task();
    uart_init();
    enable_mini_uart_interrupt();
    enable_interrupt();
    init_cpio_addr();
    init_timer();
    core_timer_enable();
    uart_puts("Welcome!\r\n");
    // disable_interrupt();
    // uart_getc(); //暫停
    // execfile("exception.img");
}

void shell_cmd(char *cmd){
    if(strcmp(cmd, "help")){
        uart_puts("help\t: print this help menu\r\n");
        uart_puts("hello\t: print Hello World!\r\n");
        uart_puts("devinfo\t: print device info\r\n");
        uart_puts("reboot\t: reboot the device\r\n");
        uart_puts("ls\t: list files\r\n");
        uart_puts("cat\t: display content of a file\r\n");
        uart_puts("exec\t: execute img file\r\n");
        uart_puts("showtime\t: show time after booting\r\n");
        uart_puts("setTimeout\t: show message after seconds\r\n");
    }
    else if(strcmp(cmd, "hello")){
        uart_puts("Hello World!\r\n");
    }
    else if(strcmp(cmd, "devinfo")){
        get_board_revision();
        get_memory_info();
    }
    else if(strcmp(cmd, "reboot")){
        uart_puts("Waiting for reboot ...\r\n");
        reset(100);
    }
    else if(strcmp(cmd, "ls")){
        ls();
    }
    else if(strcmp(cmd, "cat")){
        uart_puts("Filename: \r\n");
        char filepath[128];
        char in;
        int i = 0;
        while(1) {
            // in = uart_getc();
            in = uart_async_getc();

            if(in == '\n'){
                uart_puts("\r\n");
                cat(filepath);
                break;
            }
            else{
                filepath[i] = in;
                uart_send(in);
                i++;
            }
        }
    }
    else if(strcmp(cmd, "exec")){
        uart_puts("Filename: \r\n");
        char filepath[128];
        char in;
        int i = 0;
        while(1) {
            // in = uart_getc();
            in = uart_async_getc();

            if(in == '\n'){
                uart_puts("\r\n");
                execfile(filepath);
                break;
            }
            else{
                filepath[i] = in;
                uart_send(in);
                i++;
            }
        }
    }
    else if(strcmp(cmd, "showtime")){
        add_timer(show_time_every_two_seconds, 2, "");
    }
    else if(strncmp(cmd, "setTimeout", sizeof("setTimeout") - 1)){
        char *message = strchr(cmd, ' ') + 1;
        char *end_message = strchr(message, ' ');
        *end_message = '\0';
        char *seconds = end_message + 1;
        add_timer(uart_puts, atoi(seconds), message);
    }
    else if(strcmp(cmd, "demo1")){
        printf("exec exception.img\r\n");
        execfile("exception.img");
    }
    else if(strcmp(cmd, "demo2")){
        printf("set three timers: 5s 10s 15s\r\n");
        add_timer(uart_puts, 15, "15\r\n");
        add_timer(uart_puts, 10, "10\r\n");
        add_timer(uart_puts, 5, "5\r\n");
    }
    else{
        uart_puts("Command not found.\r\n");
    }
    
}

void shell_run(){
    char in;
    char cmd[CMD_LEN];
    int end = 0;
    uart_puts("> ");
    while(1) {
        // in = uart_getc();
        in = uart_async_getc();

        if(in == '\n'){
            cmd[end] = '\0';
            uart_puts("\r\n");
            shell_cmd(cmd);
            uart_puts("> ");
            end = 0;
        }
        else{
            cmd[end] = in;
            uart_send(in);
            end++;
        }
    }
}
