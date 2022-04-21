#ifndef _UART_H
#define _UART_H

void uart_init();
void uart_send(unsigned int c);
char uart_getc();
char uart_getc_raw();
void uart_puts(char *s);
void uart_puth(unsigned int d);
void printf(char *fmt, ...);

#endif