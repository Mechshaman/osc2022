#include "utils.h"

/* Align 'n' up to the value 'align', which must be a power of two. */
unsigned long align_up(unsigned long n, unsigned long align)
{
    return (n + align - 1) & (~(align - 1));
}