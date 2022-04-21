#include "task.h"
#include "list.h"
#include "smalloc.h"
#include "exception.h"

int curr_task_priority = 9999;
struct list_head *task_list;

void init_task(){
    task_list = simple_malloc(sizeof(task_t));
    INIT_LIST_HEAD(task_list);
}

void add_task(void *func, unsigned long long priority){
    task_t *new_task = simple_malloc(sizeof(task_t));

    new_task->priority = priority;
    new_task->task_func = func;
    INIT_LIST_HEAD(&new_task->listhead);

    struct list_head *curr;

    disable_interrupt(); // critical section

    // sort
    list_for_each(curr, task_list)
    {
        if (((task_t *)curr)->priority > new_task->priority)
        {
            list_add(&new_task->listhead, curr->prev);
            break;
        }
    }

    if (list_is_head(curr, task_list))
    {
        list_add_tail(&new_task->listhead, task_list);
    }

    enable_interrupt();
}

void run_task_list(){
    while (!list_empty(task_list))
    {
        disable_interrupt();  // critical section
        task_t *the_task = (task_t *)task_list->next;

        if (curr_task_priority <= the_task->priority)
        {
            enable_interrupt();
            break;
        }
        list_del_entry((struct list_head *)the_task);
        int prev_task_priority = curr_task_priority;
        curr_task_priority = the_task->priority;
        enable_interrupt();

        run_task(the_task);

        disable_interrupt(); // critical section
        curr_task_priority = prev_task_priority;
        enable_interrupt();
    }
}

void run_task(task_t* task) {
    ((void (*)())task->task_func)();
}