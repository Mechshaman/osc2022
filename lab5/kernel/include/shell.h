#ifndef _SHELL_H
#define _SHELL_H 

void shell_init();
void shell_run();
void shell_cmd(char *);
void foo();
void foo2();
void foo3();
void foo4();
int user_get_pid();
void user_read(char buf[], int size);
void user_write(char buf[], int size);
void user_fork();
void user_kill(int pid);

#endif