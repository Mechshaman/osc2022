#include "string.h"

int strcmp(char *s1, char *s2){
    for(int i = 0; ; i++) {
        if(s1[i] != s2[i]) {
            return 0;
        }
        else if(s1[i] == '\0' && s2[i] == '\0') {
            return 1;
        }
    }
}

int strncmp(char *s1, char *s2, unsigned long n){
    for(int i = 0; i < n; i++) {
        if(s1[i] != s2[i]) {
            return 0;
        }
        else if(s1[i] == '\0' && s2[i] == '\0') {
            return 1;
        }
    }
    return 1;
}

unsigned long long strlen(const char *str)
{
  unsigned long long count = 0;
  while((unsigned char)*str++)count++;
  return count;
}

char* strcpy(char *dest, const char *src)
{
  return memcpy(dest, src, strlen (src) + 1);
}

char* memcpy(void *dest, const void *src, unsigned long long len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
  {
    *d++ = *s++;
  }
  return dest;
}

char* strchr(register const char *s, int c)
{
  do {
    if (*s == c)
      {
        return (char*)s;
      }
  } while (*s++);
  return (0);
}

int atoi(char* str)
{
  int res = 0;

  for (int i = 0; str[i] != '\0'; ++i)
  {
    if(str[i] > '9' || str[i] < '0')return res;
    res = res * 10 + str[i] - '0';
  }

  return res;
}