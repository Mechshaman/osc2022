#ifndef REGISTERS_H
#define REGISTERS_H

// Architectural Feature Access Control Register
#define CPACR_EL1_FPEN      (0b11 << 20)
#define CPACR_EL1_VALUE     (CPACR_EL1_FPEN)

#endif