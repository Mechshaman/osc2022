#include "syscall.h"
#include "uart.h"
#include "sched.h"
#include "mailbox.h"
#include "exception.h"

int getpid(trapframe_t *trapframe) {
    trapframe->x0 = curr_thread->pid;
    return curr_thread->pid;
}

size_t uartread(trapframe_t *trapframe, char buf[], size_t size) {
    int i = 0;
    for (int i = 0; i < size;i++)
    {
        // buf[i] = uart_getc();
        buf[i] = uart_async_getc();
    }
    trapframe->x0 = i;
    return i;
}

size_t uartwrite(trapframe_t *trapframe, const char buf[], size_t size) {
    int i = 0;
    for (int i = 0; i < size; i++)
    {
        uart_send(buf[i]);
    }
    trapframe->x0 = i;
    return i;
}

int exec(trapframe_t *trapframe, const char *name, char *const argv[]) {
    curr_thread->datasize = get_file_size((char*)name);
    char *new_data = get_file_start((char *)name);
    for (unsigned int i = 0; i < curr_thread->datasize;i++) {
        curr_thread->data[i] = new_data[i];
    }

    trapframe->elr_el1 = (unsigned long)curr_thread->data;
    trapframe->sp_el0 = (unsigned long)curr_thread->stack_alloced_ptr + USTACK_SIZE;
    trapframe->x0 = 0;
    return 0;
}

int fork(trapframe_t *trapframe) {
    disable_interrupt();

    thread_t *newthread = thread_create(curr_thread->data);

    for (int i = 0; i <= SIGNAL_MAX;i++) {
        newthread->signal_handler[i] = curr_thread->signal_handler[i];
    }

    newthread->datasize = curr_thread->datasize;
    int parent_pid = curr_thread->pid;
    thread_t *parent_thread_t = curr_thread;

    for (int i = 0; i < USTACK_SIZE; i++)
    {
        newthread->stack_alloced_ptr[i] = curr_thread->stack_alloced_ptr[i];
    }

    for (int i = 0; i < KSTACK_SIZE; i++)
    {
        newthread->kernel_stack_alloced_ptr[i] = curr_thread->kernel_stack_alloced_ptr[i];
    }
    
    store_context(get_current());
    
    // printf("curr_thread ustack: %x\r\n", curr_thread->stack_alloced_ptr);
    // printf("curr_thread kstack: %x\r\n", curr_thread->kernel_stack_alloced_ptr);
    // printf("curr_thread context.sp: %x\r\n", curr_thread->context.sp);
    // printf("newthread ustack: %x\r\n", newthread->stack_alloced_ptr);
    // printf("newthread kstack: %x\r\n", newthread->kernel_stack_alloced_ptr);
    // printf("newthread context.sp: %x\r\n", newthread->context.sp);

    // int sp;
    // asm("mov %0, sp"
    //     : "=r"(sp));
    // printf("sp : %x\r\n", sp);

    if( parent_pid != curr_thread->pid)
    {
        trapframe = (trapframe_t*)((char *)trapframe + (unsigned long)newthread->kernel_stack_alloced_ptr - (unsigned long)parent_thread_t->kernel_stack_alloced_ptr);
        trapframe->sp_el0 += newthread->stack_alloced_ptr - parent_thread_t->stack_alloced_ptr;
        trapframe->x0 = 0;
        
        // printf("child \r\n");
        return 0;
    }
    else {
        newthread->context = curr_thread->context;
        newthread->context.fp += newthread->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr;
        newthread->context.sp += newthread->kernel_stack_alloced_ptr - curr_thread->kernel_stack_alloced_ptr;

        trapframe->x0 = newthread->pid;
        // printf("parent \r\n");

        enable_interrupt();
        return newthread->pid;
    }
}

void exit(trapframe_t *trapframe, int status) {
    thread_exit();
}

int mbox_call(trapframe_t *trapframe, unsigned char ch, unsigned int *mbox) {
    disable_interrupt();

    unsigned long r = (((unsigned long)((unsigned long)mbox) & ~0xF) | (ch & 0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");} while (*MAILBOX_STATUS & MAILBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MAILBOX_WRITE = r;
    /* now wait for the response */
    while (1)
    {
        /* is there a response? */
        do
        {
            asm volatile("nop");
        } while (*MAILBOX_STATUS & MAILBOX_EMPTY);
        /* is it a response to our message? */
        if (r == *MAILBOX_READ)
        {
            /* is it a valid successful response? */
            trapframe->x0 = (mbox[1] == MAILBOX_RESPONSE);
            return mbox[1] == MAILBOX_RESPONSE;
        }
    }

    trapframe->x0 = 0;
    
    enable_interrupt();
    return 0;
}

void kill(trapframe_t *trapframe, int pid) {
    disable_interrupt();

    struct list_head *curr;

    list_for_each(curr, run_queue)
    {
        if (((thread_t *)curr)->pid == pid)
        {
            ((thread_t *)curr)->iszombie = 1;
            break;
        }
    }
    
    enable_interrupt();
}

void signal_register(int signal, void (*handler)()) {
    if (signal > SIGNAL_MAX || signal < 0) return;

    curr_thread->signal_handler[signal] = handler;
    // printf("pid : %d\r\n", curr_thread->pid);
    // printf("register : %d\r\n", signal);
    // printf("signal_handler : %x\r\n", handler);
    // printf("handler : %x\r\n", curr_thread->signal_handler[signal]);
}

void signal_kill(int pid, int signal) {
    disable_interrupt();
    // printf("kill : %d\r\n", signal);

    struct list_head *curr;

    list_for_each(curr, run_queue)
    {
        if (((thread_t *)curr)->pid == pid)
        {
            ((thread_t *)curr)->signal = signal;
            break;
        }
    }

    enable_interrupt();
}

void sigreturn(trapframe_t *trapframe) {
    unsigned long signal_ustack = trapframe->sp_el0 % USTACK_SIZE == 0 ? trapframe->sp_el0 - USTACK_SIZE : trapframe->sp_el0 & (~(USTACK_SIZE - 1));
    kfree((char*)signal_ustack);
    load_context(&curr_thread->signal_saved_context);
}