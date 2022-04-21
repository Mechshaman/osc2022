#include "uart.h"
#include "gpio.h"
#include "sprintf.h"
#include "exception.h"

// get address from linker
extern volatile unsigned char _end;

char read_buf[MAX_BUF_SIZE];
char write_buf[MAX_BUF_SIZE];
int read_buf_read, read_buf_write;
int write_buf_read, write_buf_write;

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

    /* initialize UART */
    *AUX_ENABLE |= 1;   // Enable mini UART
    *AUX_MU_CNTL = 0;    // Disable TX, RX during configuration
    *AUX_MU_IER = 0;     // Disable interrupt
    *AUX_MU_LCR = 3;     // Set the data size to 8 bit
    *AUX_MU_MCR = 0;     // Don't need auto flow control
    *AUX_MU_BAUD = 270;  // Set baud rate to 115200
    *AUX_MU_IIR = 6;     // No FIFO

    /* map UART1 to GPIO pins */
    r =* GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // gpio14, gpio15 innitial
    r |= (2 << 12) | (2 << 15);    // alt5
    *GPFSEL1 = r;
    *GPPUD = 0;            // enable pins 14 and 15
    r = 150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    r = 150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        // flush GPIO setup
    *AUX_MU_CNTL = 3;      // enable Tx, Rx
    
    while((*AUX_MU_LSR&0x01))*AUX_MU_IO; //clean rx data

    read_buf_read = 0;
    read_buf_write = 0;
    write_buf_read = 0;
    write_buf_write = 0;
}

/**
 * Send a character
 */
void uart_send(char c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR & 0x20)); // This bit is set if the transmit FIFO can accept at least one byte. 
    /* write the character to the buffer */
    *AUX_MU_IO = c;
}

/**
 * Receive a character
 */
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR & 0x01)); // This bit is set if the receive FIFO holds at least 1 symbol. 
    /* read it and return */
    r = (char)(*AUX_MU_IO);
    /* convert carrige return to newline */
    return r == '\r'?'\n':r;
}

char uart_getc_raw() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR & 0x01)); // This bit is set if the receive FIFO holds at least 1 symbol. 
    /* read it and return */
    r = (char)(*AUX_MU_IO);
    return r;
}

/**
 * Display a string
 */
void uart_puts(char *s) {
    while(*s) {
        /* convert newline to carrige return + newline */
        if(*s == '\n')
            uart_send('\r');
            // uart_async_send('\r');
        uart_send(*s++);
        // uart_async_send(*s++);
    }
}

void uart_puts_n(char *s, unsigned long n) {
    for(int i=0; i<n;i++){
        if(*s == '\n')
            uart_send('\r');
            // uart_async_send('\r');
        uart_send(*s++);
        // uart_async_send(*s++);
    }
}

void uart_puth(unsigned int d) {
    unsigned int n;
    int c;
    int print = 0;
    uart_puts("0x");
    for(c = 28; c >= 0; c-= 4) {
        // get highest tetrad
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x37 : 0x30;
        if(n != '0'){
            print = 1;
        }
        if(print){
            uart_send(n);
            // uart_async_send(n);
        }
    }
    if(print == 0){
        uart_send('0');
        // uart_async_send('0');
    }
}

void printf(char *fmt, ...)
{
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[MAX_BUF_SIZE];
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    char *s = (char *)buf;
    // use sprintf to format our string
    int count = vsprintf(s, fmt, args);
    // print out as usual
    while (*s)
    {
        uart_send(*s++);
        // uart_async_send(*s++);
    }
    __builtin_va_end(args);
}

void enable_mini_uart_interrupt()
{
    enable_read_interrupt();
    enable_write_interrupt();
    *ENABLE_IRQS_1 |= IRQ_AUX_INT; // interrupts from Aux int
}

void disable_mini_uart_interrupt()
{
    disable_read_interrupt();
    disable_write_interrupt();
}

void enable_read_interrupt()
{
    *AUX_MU_IER |= 1;
}

void enable_write_interrupt()
{
    *AUX_MU_IER |= 2;
}

void disable_read_interrupt()
{
    *AUX_MU_IER &= ~(1);
}

void disable_write_interrupt()
{
    *AUX_MU_IER &= ~(2);
}

char uart_async_getc()
{
    while (read_buf_read == read_buf_write) // buffer empty
        enable_read_interrupt();

    // critical section
    disable_interrupt();
    char r = read_buf[read_buf_read++];

    if (read_buf_read >= MAX_BUF_SIZE)
        read_buf_read = 0;

    enable_interrupt();

    enable_read_interrupt();

    return r;
}

void uart_async_send(char c)
{
    while ((write_buf_write + 1) % MAX_BUF_SIZE == write_buf_read) // buffer full
    {
        enable_write_interrupt();
    }

    disable_interrupt();
    write_buf[write_buf_write++] = c;
    if (write_buf_write >= MAX_BUF_SIZE)
        write_buf_write = 0;
    
    enable_interrupt();

    enable_write_interrupt();
}

void uart_read_interrupt_handler()
{
    if ((read_buf_write + 1) % MAX_BUF_SIZE == read_buf_read) // read buffer full
    {
        disable_read_interrupt();
        return;
    }
    read_buf[read_buf_write++] = uart_getc();
    if (read_buf_write >= MAX_BUF_SIZE)
        read_buf_write = 0;

    enable_read_interrupt();
}

void uart_write_interrupt_handler() //can write
{
    if (write_buf_read == write_buf_write) // buffer empty
    {
        disable_write_interrupt(); // disable w_interrupt to prevent interruption without any async output
        return;
    }
    uart_send(write_buf[write_buf_read++]);
    if (write_buf_read >= MAX_BUF_SIZE)
        write_buf_read = 0;
    enable_write_interrupt();
}