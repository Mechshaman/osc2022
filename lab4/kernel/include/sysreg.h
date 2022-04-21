#ifndef _SYSREG_H_
#define _SYSREG_H_

#define read_sysreg(r) ({                       \
    unsigned long __val;                        \
    asm volatile("mrs %0, " #r : "=r" (__val)); \
    __val;                                      \
})

#define write_sysreg(r, __val) ({                  \
	asm volatile("msr " #r ", %0" :: "r" (__val)); \
})

#endif