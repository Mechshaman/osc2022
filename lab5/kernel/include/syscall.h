#ifndef SYSCALL_H
#define SYSCALL_H

#include "exception.h"
#include "stddef.h"

int getpid(trapframe_t *trapframe);
size_t uartread(trapframe_t *trapframe, char buf[], size_t size);
size_t uartwrite(trapframe_t *trapframe, const char buf[], size_t size);
int exec(trapframe_t *trapframe, const char *name, char *const argv[]);
int fork(trapframe_t *trapframe);
void exit(trapframe_t *trapframe, int status);
int mbox_call(trapframe_t *trapframe, unsigned char ch, unsigned int *mbox);
void kill(trapframe_t *trapframe, int pid);
void signal_register(int signal, void (*handler)());
void signal_kill(int pid, int signal);
void sigreturn(trapframe_t *trapframe);

#endif