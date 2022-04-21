#include "smalloc.h"
#include "exec.h"

int exec(char* data){

    char* stackEL0 = simple_malloc(STACKEL0_SIZE);

    asm("mov x1, 0x3c0\n\t"
        // "msr spsr_el1, xzr\n\t"
        "msr spsr_el1, x1\n\t"
        "msr elr_el1, %0\n\t"
        "msr sp_el0, %1\n\t"
        "eret\n\t"
        :: "r" (data),
           "r" (stackEL0+STACKEL0_SIZE));

    return 0;
}