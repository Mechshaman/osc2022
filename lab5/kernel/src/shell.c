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
#include "mm.h"
#include "sched.h"
#include "exec.h"
#include "sysreg.h"

#define CMD_LEN 128

void init_cpio_addr()
{
    fdt_traverse(initramfs_callback);
}

void shell_init(){
    init_allocator();
    init_task();
    uart_init();
    enable_mini_uart_interrupt();
    enable_interrupt();
    init_cpio_addr();
    init_thread_queue();
    init_timer();
    core_timer_enable();
    uart_puts("Welcome!\r\n");
    // disable_interrupt();
    // uart_getc(); //暫停
    // execfile("exception.img");

    // thread_create(foo);
    // thread_create(foo);
    // thread_create(foo4);
    // thread_create(foo);
    // thread_create(foo);
    // add_timer_bytick(schedule_timer, 0, "");
    // exec_thread(foo2);
    // uart_puts("XXXXXXXXXXXXXXXXX\r\n");
    // idle();
    // exec_origin(foo);

    execfile("syscall.img");
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
    else if(strcmp(cmd, "demo3")){
        test_buddy_alloc();
    }
    else if(strcmp(cmd, "demo4")){
        test_dynamic_alloc();
    }
    else if(strcmp(cmd, "ttt")){
        // thread_create(foo);
        // for(int i = 0; i < N; ++i) { // N should > 2
        //     thread_create(foo);
        // }
        // idle();
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

void foo() {
    // for(int i = 0; i < 10; ++i) {
    //     printf("Thread id: %d %d\n", current_thread().id(), i);
    //     delay(1000000);
    //     schedule();
    // }
    // printf("%d\r\n", curr_thread->pid);
    while(1) {
        printf("%d\r\n", curr_thread->pid);
        // uart_puts("123\r\n");
        // asm("mov x1, 0x3c0\n\t"
        // "msr spsr_el1, x1\n\t");
        // add_timer(uart_puts, 5, "5\r\n");
    }
    // printf("123\r\n");
    // thread_exit();
    // asm("mov x8, 7\n\t"
    // "svc 5\n\t"
    // );
}

void foo2() {

    user_kill(4);

    // int a;

    // int sp;
    // asm("mov %0, sp"
    //     : "=r"(sp));
    // printf("sp : %x\r\n", sp);

    // user_fork();
    // a = user_get_pid();
    // printf("pid : %d\r\n", a);

    // user_fork();
    // a = user_get_pid();
    // printf("pid : %d\r\n", a);


    // a = user_get_pid();
    // printf("pid : %d\r\n", a);


    // char buf[2];
    // int size = 2;

    // user_read(buf, size);
    // uart_puts("123\r\n");
    // uart_puts(buf);
    // uart_puts("\r\n");
    // uart_puts("123\r\n");
    // user_write(buf, size);
    // uart_puts("\r\n");
    // uart_puts("123\r\n");

    while(1) {
        
    }

    // while(1) {
    //     // uart_puts("456\r\n");
    //     asm("mov x1, 0x3c0\n\t"
    //     "msr spsr_el1, x1\n\t");
    // }

    // printf("%d\r\n", current_thread_pid());
    // uart_getc();
    // thread_exit();
}

void foo3() {
    uart_puts("foo3\r\n");
    exec_origin(foo);
}

void foo4() {
    uart_puts("foo4\r\n");
    exec_origin(foo2);
}

int user_get_pid() {

    int tmp;

    asm("mov x8, 0\n\t"
        "svc 0\n\t"
        "mov %0, x0"
        : "=r"(tmp));
    
    return tmp;
}

void user_read(char buf[], int size) {

    asm("mov x0, %0\n\t"
        "mov x1, %1\n\t"
        "mov x8, 1\n\t"
        "svc 0\n\t"
        ::   "r" (buf),
            "r"(size));
}

void user_write(char buf[], int size) {

    asm("mov x0, %0\n\t"
        "mov x1, %1\n\t"
        "mov x8, 2\n\t"
        "svc 0\n\t"
        ::   "r" (buf),
            "r"(size));
}

void user_fork() {
    asm("mov x8, 4\n\t"
        "svc 0\n\t");
}

void user_kill(int pid) {
    asm("mov x0, %0\n\t"
        "mov x8, 7\n\t"
        "svc 0\n\t"
        ::   "r" (pid));
}