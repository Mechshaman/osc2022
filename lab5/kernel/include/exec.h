#ifndef EXEC_H
#define EXEC_H

#define STACKEL0_SIZE 0x10000

int exec_origin(char* file);
int exec_thread(char* data, unsigned int filesize);

#endif