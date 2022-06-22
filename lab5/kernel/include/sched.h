#ifndef SCHED_H
#define SCHED_H

#include "list.h"
#include "signal.h"

#define USTACK_SIZE 0x10000
#define KSTACK_SIZE 0x10000

extern void switch_to(void *curr_context, void *next_context);
extern void *get_current();

typedef struct thread_context
{
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;
    unsigned long lr;
    unsigned long sp;
} thread_context_t;

typedef struct thread
{
    list_head_t listhead;
    thread_context_t context;
    int pid;
    char *data;
    unsigned int datasize;
    int iszombie;
    char* stack_alloced_ptr;
    char *kernel_stack_alloced_ptr;
    int signal;
    void (*signal_handler[SIGNAL_MAX+1])();
    thread_context_t signal_saved_context;
} thread_t;

thread_t *curr_thread;
list_head_t *run_queue;
list_head_t *wait_queue;

void init_thread_queue();
thread_t *thread_create(void *start);
void thread_exit();
void schedule();
void kill_zombies();
void idle();
int current_thread_pid();
void schedule_timer();

#endif