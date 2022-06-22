#include "mm.h"
#include "exec.h"
#include "sched.h"
#include "uart.h"
#include "timer.h"

int exec_origin(char* data) {

    char* stackEL0 = kmalloc(STACKEL0_SIZE);

    asm("msr spsr_el1, xzr\n\t"
        // "mov x1, 0x3c0\n\t"
        // "msr spsr_el1, x1\n\t"
        "msr elr_el1, %0\n\t"
        "msr sp_el0, %1\n\t"
        "eret\n\t"
        :: "r" (data),
           "r" (stackEL0+STACKEL0_SIZE));

    return 0;
}

int exec_thread(char* data, unsigned int filesize) {
    thread_t *thread = thread_create(data);
    thread->data = kmalloc(filesize);
    thread->datasize = filesize;
    thread->context.lr = (unsigned long)thread->data;
    
    for (int i = 0; i < filesize;i++) {
        thread->data[i] = data[i];
    }

    curr_thread = thread;

    add_timer(schedule_timer, 1, "");

    // printf("exec_thread\r\n");

    asm("msr tpidr_el1, %0\n\t"
        "msr elr_el1, %1\n\t"
        "msr spsr_el1, xzr\n\t"
        "msr sp_el0, %2\n\t"
        "mov sp, %3\n\t"
        "eret\n\t" ::"r"(&thread->context),"r"(thread->context.lr), "r"(thread->context.sp), "r"(thread->kernel_stack_alloced_ptr + KSTACK_SIZE));

    return 0;
}