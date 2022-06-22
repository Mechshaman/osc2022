#ifndef _TASK_H
#define _TASK_H

#include "list.h"

#define PRIORITY_UART 1
#define PRIORITY_TIMER 0

typedef struct task
{
    struct list_head listhead;

    unsigned long long priority;

    void *task_func;

} task_t;

void init_task();
void add_task(void *func, unsigned long long priority);
void run_task_list();
void run_task(task_t* task);
#endif