#ifndef _TIMER_H
#define _TIMER_H

#include "list.h"

#define CORE0_TIMER_IRQ_CTRL ((volatile unsigned int *)(0x40000040))

typedef struct timer_event
{
    struct list_head listhead;

    unsigned long long interrupt_time;

    void *callback;

    char *args;
} timer_event_t;

void init_timer();
void core_timer_enable();
void core_timer_disable();
void core_timer_handler();
void add_timer(void *callback, unsigned long long timeout, char *args);

void start_timer_event(timer_event_t *timer_event);
void show_time_every_two_seconds();
unsigned long long get_interrupt_time(unsigned long long second);
void set_timer_interrupt_second(unsigned long long expired_time);
void set_timer_interrupt(unsigned long long tick);

#define STR(x) #x
#define XSTR(s) STR(s)

#endif