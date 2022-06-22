#include "sched.h"
#include "mm.h"
#include "uart.h"
#include "exception.h"

int thread_pid = 0;

void init_thread_queue(){
    disable_interrupt();

    run_queue = kmalloc(sizeof(list_head_t));
    wait_queue = kmalloc(sizeof(list_head_t));
    INIT_LIST_HEAD(run_queue);
    INIT_LIST_HEAD(wait_queue);

    thread_t* idlethread = thread_create(idle);
    curr_thread = idlethread;

    enable_interrupt();
}

thread_t *thread_create(void *start){
    disable_interrupt();

    thread_t *newthread;
    newthread = kmalloc(sizeof(thread_t));
    newthread->context.lr = (unsigned long long)start;
    newthread->pid = thread_pid;
    thread_pid++;
    newthread->iszombie = 0;
    newthread->stack_alloced_ptr = kmalloc(USTACK_SIZE);
    newthread->kernel_stack_alloced_ptr = kmalloc(KSTACK_SIZE);
    newthread->context.sp = (unsigned long long )newthread->stack_alloced_ptr + USTACK_SIZE;
    newthread->context.fp = newthread->context.sp;

    newthread->signal = -1;
    for (int i = 0; i < SIGNAL_MAX;i++)
    {
        newthread->signal_handler[i] = signal_default_handler;
    }

    list_add(&newthread->listhead, run_queue);
    enable_interrupt();
    return newthread;
}

void thread_exit(){
    curr_thread->iszombie = 1;
    schedule();
}

void schedule(){    
    disable_interrupt();

    do{
        curr_thread = (thread_t *)curr_thread->listhead.next;
    } while (list_is_head(&curr_thread->listhead, run_queue) || curr_thread->iszombie);

    switch_to(get_current(), &curr_thread->context);
    // uart_puts("switch to\r\n");

    enable_interrupt();
}

void kill_zombies(){
    disable_interrupt();

    list_head_t *curr;
    list_for_each(curr,run_queue)
    {
        if (((thread_t *)curr)->iszombie)
        {
            // uart_puts("kill zombie\r\n");
            list_del_entry(curr);
            kfree(((thread_t *)curr)->stack_alloced_ptr);
            kfree(((thread_t *)curr)->kernel_stack_alloced_ptr);
            kfree((thread_t *)curr);
        }
    }

    enable_interrupt();
}

void idle(){
    while(1)
    {
        kill_zombies();
        schedule();
    }
}

int current_thread_pid(){
    return curr_thread->pid;
}

void schedule_timer(){
    // uart_puts("add schedule timer\r\n");
    unsigned long long cntfrq_el0;
    __asm__ __volatile__("mrs %0, cntfrq_el0\n\t": "=r"(cntfrq_el0)); //tick frequency
    add_timer_bytick(schedule_timer, cntfrq_el0 >> 5, "");
    // add_timer_bytick(schedule_timer, cntfrq_el0/10, "");
}
