#ifndef _UART_H
#define _UART_H

#include "gpio.h"

/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))

#define MAX_BUF_SIZE 0x100

void uart_init();
void uart_send(char c);
char uart_getc();
char uart_getc_raw();
void uart_puts(char *s);
void uart_puts_n(char *s, unsigned long n);
void uart_puth(unsigned int d);
void printf(char *fmt, ...);
void enable_mini_uart_interrupt();
void disable_mini_uart_interrupt();
void enable_read_interrupt();
void enable_write_interrupt();
void disable_read_interrupt();
void disable_write_interrupt();
char uart_async_getc();
void uart_async_send(char c);
void uart_read_interrupt_handler();
void uart_write_interrupt_handler();

#endif