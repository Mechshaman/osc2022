#include "timer.h"
#include "sysreg.h"
#include "uart.h"
#include "smalloc.h"
#include "list.h"
#include "string.h"

struct list_head *timer_event_list;

void init_timer()
{
    timer_event_list = simple_malloc(sizeof(timer_event_t));
    INIT_LIST_HEAD(timer_event_list);
}

void core_timer_enable(){
    asm volatile(
        "mov x0, 1\n\t"
        "msr cntp_ctl_el0, x0\n\t" // enable EL1 physical timer
        // "mrs x0, cntfrq_el0\n\t"
        // "msr cntp_tval_el0, x0\n\t" // set expired time (system counter clock frequency)
    );
    *CORE0_TIMER_IRQ_CTRL |= (1<<1); // enable core 0 nCNTPNSIRQ(non-secure EL1 physical timer) IRQ control
}

void core_timer_disable()
{
    *CORE0_TIMER_IRQ_CTRL = 0;
}

void start_timer_event(timer_event_t *timer_event)
{
    list_del_entry((struct list_head *)timer_event);
    ((void (*)(char *))timer_event->callback)(timer_event->args);

    //set interrupt to next time_event if existing
    if (!list_empty(timer_event_list))
    {
        set_timer_interrupt(((timer_event_t *)timer_event_list->next)->interrupt_time);
    }
    else
    {
        // printf("Timer list empty (start_timer_event)\r\n");
        set_timer_interrupt_second(100);
    }

}

void core_timer_handler(){
    if (list_empty(timer_event_list))
    {
        // printf("Timer list empty (core_timer_handler)\r\n");
        set_timer_interrupt_second(100);
        core_timer_enable();
        return;
    }

    start_timer_event((timer_event_t *)timer_event_list->next);
    core_timer_enable();
}

void show_time_every_two_seconds(){
    unsigned long cntpct = read_sysreg(cntpct_el0);
    unsigned long cntfrq = read_sysreg(cntfrq_el0);
    unsigned long sec = cntpct/cntfrq;
    printf("%d seconds after booting...\r\n", sec);

    asm volatile(
        "mrs x0, cntfrq_el0\n\t"
        "mov x1, 2\n\t"
        "mul x0, x0, x1\n\t"
        "msr cntp_tval_el0, x0\n\t"
    );

    add_timer(show_time_every_two_seconds, 2, "");
}

void add_timer(void *callback, unsigned long long timeout, char *args)
{
    timer_event_t *the_timer_event = simple_malloc(sizeof(timer_event_t));

    the_timer_event->args = simple_malloc(strlen(args) + 1);
    strcpy(the_timer_event->args, args);

    the_timer_event->interrupt_time = get_interrupt_time(timeout);
    the_timer_event->callback = callback;
    INIT_LIST_HEAD(&the_timer_event->listhead);

    struct list_head *curr;

    // sort
    list_for_each(curr, timer_event_list)
    {
        if (((timer_event_t *)curr)->interrupt_time > the_timer_event->interrupt_time)
        {
            list_add(&the_timer_event->listhead, curr->prev);
            break;
        }
    }

    if (list_is_head(curr, timer_event_list))
    {
        list_add_tail(&the_timer_event->listhead, timer_event_list); // for the time is the biggest
    }

    set_timer_interrupt(((timer_event_t *)timer_event_list->next)->interrupt_time);
}

unsigned long long get_interrupt_time(unsigned long long second)
{
    unsigned long cntpct = read_sysreg(cntpct_el0);
    unsigned long cntfrq = read_sysreg(cntfrq_el0);

    return (cntpct + cntfrq * second);
}

void set_timer_interrupt_second(unsigned long long expired_time)
{
    asm volatile(
        "mrs x1, cntfrq_el0\n\t"
        "mul x1, x1, %0\n\t"
        "msr cntp_tval_el0, x1\n\t"
        : "=r"(expired_time));
}

void set_timer_interrupt(unsigned long long tick)
{
    asm volatile(
        "msr cntp_cval_el0, %0\n\t"
        : "=r"(tick));
}