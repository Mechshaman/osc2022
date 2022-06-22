#include "signal.h"
#include "syscall.h"
#include "sched.h"
#include "uart.h"

void run_signal(trapframe_t* trapframe) {
    if (curr_thread->signal >= SIGNAL_MAX || curr_thread->signal == -1) return;

    store_context(&curr_thread->signal_saved_context);

    // printf("pid : %d\r\n", curr_thread->pid);
    // printf("run signal : %d\r\n", curr_thread->signal);
    // printf("signal handler to run : %x\r\n", curr_thread->signal_handler[curr_thread->signal]);
    // printf("signal handler -1 : %x\r\n", curr_thread->signal_handler[curr_thread->signal-1]);
    // printf("default handler : %x\r\n", signal_default_handler);

    if (curr_thread->signal_handler[curr_thread->signal] == signal_default_handler)
    {
        // printf("default handler\r\n");
        signal_default_handler();
        return;
    }

    // printf("not default handler\r\n");
    char *temp_signal_userstack = kmalloc(USTACK_SIZE);

    asm("msr elr_el1, %0\n\t"
        "msr sp_el0, %1\n\t"
        "msr spsr_el1, %2\n\t"
        "eret\n\t" ::"r"(signal_handler_wrapper),
        "r"(temp_signal_userstack + USTACK_SIZE),
        "r"(trapframe->spsr_el1));
}

void signal_handler_wrapper() {
    (curr_thread->signal_handler[curr_thread->signal])();
    
    // printf("signal_handler_wrapper\r\n");
    curr_thread->signal = -1;

    asm("mov x8,10\n\t"
        "svc 0\n\t");
}

void signal_default_handler() {
    kill(0,curr_thread->pid);
}