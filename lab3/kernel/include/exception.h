#ifndef EXCEPTION_H
#define EXCEPTION_H

#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int*)(0x40000060))
#define INTERRUPT_SOURCE_CNTPNSIRQ (1<<1)
#define INTERRUPT_SOURCE_GPU (1<<8)

#define INTERRUPT_REGISTER_BASE 0x3F00b000
#define IRQ_PENDING_1  ((volatile unsigned int*)(INTERRUPT_REGISTER_BASE+0x204))
#define ENABLE_IRQS_1  ((volatile unsigned int*)(INTERRUPT_REGISTER_BASE+0x210))
#define IRQ_AUX_INT (1<<29)

void sync_router();
void irq_router();
void fiq_router();
void serror_router();
void enable_interrupt();
void disable_interrupt();
void show_exception_status();

#endif