#ifndef _STRING_H
#define _STRING_H 

int strcmp(char *s1, char *s2);
int strncmp(char *s1, char *s2, unsigned long n);
unsigned long long strlen(const char *str);
char* strcpy(char *dest, const char *src);
char* memcpy(void *dest, const void *src, unsigned long long len);
char* strchr(register const char *s, int c);
int atoi(char* str);

#endif