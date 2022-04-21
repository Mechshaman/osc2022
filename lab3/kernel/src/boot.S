#include "register.h"

.section ".text.boot"

.global _start

_start:
    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b
2:  // cpu id == 0
    bl from_el2_to_el1

set_exception_vector_table:
    adr x1, exception_vector_table
    msr vbar_el1, x1

    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =_start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

    // jump to C code, should not return
4:  bl      main
    // for failsafe, halt this core too
    b       1b


from_el2_to_el1:
    /* 
        在 EL1 用到 Advanced SIMD and floating-point registers 會跳 Execution
        pintf 中用到的 q0 ~ q7 是 NEON registers
        NEON registers 會 map 到 SIMD and FP destination register 的 D0-D31
    */
    ldr x1, =CPACR_EL1_VALUE
    msr CPACR_EL1, x1

    mov x1, (1 << 31) // EL1 uses aarch64
    msr hcr_el2, x1
    mov x1, 0x3c5 // EL1h (SPSel = 1) with interrupt disabled
    msr spsr_el2, x1
    msr elr_el2, lr
    eret // return to EL1