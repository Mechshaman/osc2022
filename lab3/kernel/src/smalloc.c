#include "smalloc.h"
#include "utils.h"

extern char _heap_start;
static char* top = &_heap_start;

void* simple_malloc(unsigned long size) {
    char* addr = top;
    top += align_up(size,8);
    return addr;
}