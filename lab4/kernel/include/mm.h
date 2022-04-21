#ifndef _MM_H
#define _MM_H

#include "list.h"

#define MAX_BUDDY_ORDER 10 // 2^0 ~ 2^10 => 4k to 4MB
#define MAX_POOL_ORDER 3 // 16, 32, 48, 96
// #define BUDDY_START 0x10000000
#define BUDDY_START 0
#define BUDDY_PAGE_COUNT 0x3C000

#define FRAME_BELONG_TO_OTHER -1
#define FRAME_NOT_IN_POOL -1

typedef struct frame {
    struct list_head listhead;
    int idx;
    int order; // -1 belong to others
    int inused;
    int pool_order; // -1 not in pool
} frame_t;

void* simple_malloc(unsigned long size);
void init_allocator();
void *buddy_alloc(unsigned long long size);
void release_redundant_frame(int frame_idx,int order);
void buddy_free(void *ptr);
frame_t *get_buddy(frame_t *frame);
void *dynamic_alloc(unsigned long long size);
void put_chunks_to_pool(int order);
void dynamic_free(void *ptr);
void *kmalloc(unsigned long long size);
void *kfree(void *ptr);
void memory_reserve(unsigned long long start, unsigned long long end);
void test_buddy_alloc();
void test_dynamic_alloc();
void test_dynamic_alloc();
void show_free_list(int order);
void show_pool_list(int order);

#endif