#ifndef SIGNAL_H
#define SIGNAL_H

#include "exception.h"

#define SIGNAL_MAX  64
#define SIGKILL 9

void run_signal(trapframe_t* trapframe);
void signal_handler_wrapper();
void signal_default_handler();

#endif