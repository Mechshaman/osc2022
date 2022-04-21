#include "exception.h"
#include "uart.h"
#include "timer.h"
#include "sysreg.h"
#include "task.h"

void sync_router() {
    printf("Synchronous exception\r\n");
    show_exception_status();
}

void irq_router() {
    // printf("IRQ exception\r\n");
    if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ)
    {
        // printf("IRQ exception Timer\r\n");
        // core_timer_handler();
        core_timer_disable();
        add_task(core_timer_handler, PRIORITY_TIMER);
        run_task_list();
    }
    else if(*IRQ_PENDING_1 & IRQ_AUX_INT && *CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_GPU)
    {
        if (*AUX_MU_IIR & (0b01 << 1)) // Transmit holding register empty 
        {
            // printf("IRQ exception write\r\n");
            disable_write_interrupt();
            uart_write_interrupt_handler();
            // disable_write_interrupt();
            // add_task(uart_write_interrupt_handler, PRIORITY_UART);
            // run_task_list();
        }
        else if (*AUX_MU_IIR & (0b10 << 1)) // Receiver holds valid byte
        {
            // printf("IRQ exception read\r\n");
            // disable_read_interrupt();
            // uart_read_interrupt_handler();
            disable_read_interrupt();
            add_task(uart_read_interrupt_handler, PRIORITY_UART);
            run_task_list();
        }
    }
    else{
        // printf("IRQ exception Other\r\n");
    }
}

void fiq_router() {
    printf("FIQ exception\r\n");
}

void serror_router() {
    printf("SError exception\r\n");
}

void show_exception_status(){
    unsigned long spsr = read_sysreg(spsr_el1);
	unsigned long elr = read_sysreg(elr_el1);
    unsigned long esr = read_sysreg(esr_el1);
    printf("spsr : 0x%x\r\n", spsr); // Saved Program Status Register
    printf("elr : 0x%x\r\n", elr); // Exception Link Register
    printf("esr : 0x%x\r\n", esr); // Exception Syndrome Register
}

void enable_interrupt(){
    asm volatile("msr daifclr, 0xf");
}

void disable_interrupt(){
    asm volatile("msr daifset, 0xf");
}