#include "exception.h"
#include "uart.h"
#include "timer.h"
#include "sysreg.h"
#include "task.h"
#include "sched.h"
#include "syscall.h"

void sync_router(trapframe_t *trapframe) {
    // printf("Synchronous exception\r\n");
    // show_exception_status();
    enable_interrupt();

    unsigned long long syscall_no = trapframe->x8;

    if (syscall_no == 0) {
        getpid(trapframe);
    }
    else if(syscall_no == 1) {
        uartread(trapframe,(char *) trapframe->x0, trapframe->x1);
    }
    else if (syscall_no == 2) {
        uartwrite(trapframe,(char *) trapframe->x0, trapframe->x1);
    }
    else if (syscall_no == 3) {
        exec(trapframe,(char *) trapframe->x0, (char **)trapframe->x1);
    }
    else if (syscall_no == 4) {
        fork(trapframe);
    }
    else if (syscall_no == 5) {
        exit(trapframe,trapframe->x0);
    }
    else if (syscall_no == 6) {
        mbox_call(trapframe,(unsigned char)trapframe->x0, (unsigned int *)trapframe->x1);
    }
    else if (syscall_no == 7) {
        kill(trapframe, (int)trapframe->x0);
    }
    else if (syscall_no == 8) {
        signal_register(trapframe->x0, (void (*)())trapframe->x1);
    }
    else if (syscall_no == 9) {
        signal_kill(trapframe->x0, trapframe->x1);
    }
    else if (syscall_no == 10) {
        sigreturn(trapframe);
    }
}

void irq_router(trapframe_t *trapframe) {
    // printf("IRQ exception\r\n");
    if(*CORE0_INTERRUPT_SOURCE & INTERRUPT_SOURCE_CNTPNSIRQ)
    {
        // printf("IRQ exception Timer\r\n");
        // core_timer_handler();
        core_timer_disable();
        add_task(core_timer_handler, PRIORITY_TIMER);
        run_task_list();

        if (run_queue->next->next != run_queue){
            schedule();
        }
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
    // printf("IRQ exception end\r\n");

    if ((trapframe->spsr_el1 & 0b1100) == 0)
    {
        // printf("run_signal\r\n");
        run_signal(trapframe);
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

void enable_interrupt() {
    asm volatile("msr daifclr, 0xf");
}

void disable_interrupt() {
    asm volatile("msr daifset, 0xf");
}