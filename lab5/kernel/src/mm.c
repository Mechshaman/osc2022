#include "mm.h"
#include "list.h"
#include "uart.h"
#include "utils.h"
#include "cpio.h"

static frame_t *frame_array;
static list_head_t free_list[MAX_BUDDY_ORDER + 1];
static list_head_t pool_list[MAX_POOL_ORDER + 1];
static int pool_size[MAX_POOL_ORDER + 1];

extern char _heap_start;
static char* heap_top = &_heap_start;
extern char _end;
static char* kernel_end = &_end;

void* simple_malloc(unsigned long size) {
    char* addr = heap_top;
    heap_top += align_up(size,8);
    return addr;
}

void init_allocator() {
    frame_array = simple_malloc(BUDDY_PAGE_COUNT * sizeof(frame_t));

    for (int i = 0; i <= MAX_BUDDY_ORDER ; i++) {
        INIT_LIST_HEAD(&free_list[i]);
    }

    for (int i = 0; i < 0x3c000; i++) {
        INIT_LIST_HEAD(&frame_array[i].listhead);
        frame_array[i].idx = i;
        frame_array[i].inused = 0;
        frame_array[i].pool_order = FRAME_NOT_IN_POOL;

        if (i % (1 << MAX_BUDDY_ORDER) == 0) {
            frame_array[i].order = MAX_BUDDY_ORDER;
            list_add(&frame_array[i].listhead, &free_list[MAX_BUDDY_ORDER]);
        }
        else {
            frame_array[i].order = FRAME_BELONG_TO_OTHER;
        }
    }

    pool_size[0] = 16;
    pool_size[1] = 32;
    pool_size[2] = 48;
    pool_size[3] = 96;

    for (int i = 0; i <= MAX_POOL_ORDER ; i++) {
        INIT_LIST_HEAD(&pool_list[i]);
    }

    // printf("reserve Spin tables : 0x0000 ~ 0x1000\r\n");
    memory_reserve(0x0000, 0x1000);
    // printf("reserve kernel : 0x80000 ~ 0x%x\r\n", kernel_end);
    memory_reserve(0x80000, (unsigned long long)kernel_end);
    // printf("reserve heap : 0x%x ~ 0x%x\r\n", &_heap_start, heap_top);
    memory_reserve((unsigned long long)&_heap_start, (unsigned long long)heap_top);
    // printf("reserve cpio : 0x%x ~ 0x%x\r\n", cpio_addr, cpio_end);
    memory_reserve(cpio_addr, cpio_end);
}

void *buddy_alloc(unsigned long long size) {
    int return_order;
    for (int i = 0; i <= MAX_BUDDY_ORDER; i++) {
        if (size <= (0x1000 << i)) {
           return_order = i;
           break;
        }
    }

    frame_t *frame_ptr;
    int now_order;
    for (int i = return_order; i <= MAX_BUDDY_ORDER; i++) {
        if (!list_empty(&free_list[i])) {
            frame_ptr = (frame_t *)free_list[i].next;
            frame_ptr->order = return_order;
            frame_ptr->inused = 1;
            list_del_entry((struct list_head *)frame_ptr);
            now_order = i;
            break;
        }
    }

    for (int i = now_order; i > return_order; i--) {
        int split_frame_idx = frame_ptr->idx + (1 << (i - 1));
        release_redundant_frame(split_frame_idx, i - 1);
    }

    return (void *)BUDDY_START + (0x1000 * frame_ptr->idx);
}

void release_redundant_frame(int frame_idx,int order) {
    frame_array[frame_idx].order = order;
    list_add(&frame_array[frame_idx].listhead, &free_list[order]);
    // printf("add frame idx 0x%x to free list order %d\r\n",frame_idx,frame_array[frame_idx].order);
}

void buddy_free(void *ptr) {
    frame_t *frame_ptr = &frame_array[((unsigned long long)ptr - BUDDY_START) >> 12];
    frame_ptr->inused = 0;
    
    for (int i = frame_ptr->order; i <= MAX_BUDDY_ORDER; i++) {
        frame_t *buddy_frame = get_buddy(frame_ptr);
        if(buddy_frame->inused == 1 || buddy_frame->order != frame_ptr->order) { break; }
        // printf("combine frame idx 0x%x and frame idx 0x%x\r\n",frame_ptr->idx,buddy_frame->idx);
        list_del_entry((struct list_head *)buddy_frame);
        if (buddy_frame->idx > frame_ptr->idx) {
            buddy_frame->order = FRAME_BELONG_TO_OTHER;
        }
        else {
            frame_ptr->order = FRAME_BELONG_TO_OTHER;
            frame_ptr = buddy_frame;
        }
        frame_ptr->order += 1;
        // printf("new frame idx 0x%x and frame order %d\r\n",frame_ptr->idx,frame_ptr->order);
    }

    list_add(&frame_ptr->listhead, &free_list[frame_ptr->order]);
}

frame_t *get_buddy(frame_t *frame) {
    return &frame_array[frame->idx ^ (1 << frame->order)];
}

void *dynamic_alloc(unsigned long long size) {
    int order;
    for (int i = 0; i <= MAX_POOL_ORDER; i++) {
        if (size <= pool_size[i]) {
           order = i;
           break;
        }
    }

    if (list_empty(&pool_list[order])) {
        put_chunks_to_pool(order);
    }

    list_head_t *ptr = (frame_t *)pool_list[order].next;
    list_del_entry((struct list_head *)ptr);
    frame_t *frame_ptr;

    return ptr;
}

void put_chunks_to_pool(int order) {
    void *page_ptr = buddy_alloc(0x1000);

    frame_t *pageframe_ptr = &frame_array[((unsigned long long)page_ptr - BUDDY_START) >> 12];
    pageframe_ptr->pool_order = order;

    int chunk_size = pool_size[order];
    for (int i = 0; i < 0x1000; i += chunk_size)
    {
        list_head_t *chunk = (list_head_t *)(page_ptr + i);
        list_add(chunk, &pool_list[order]);
    }
}

void dynamic_free(void *ptr) {
    list_head_t *chunk = (list_head_t *)ptr;
    frame_t *frame_ptr = &frame_array[((unsigned long long)ptr - BUDDY_START) >> 12];
    list_add(chunk, &pool_list[frame_ptr->pool_order]);
}

void *kmalloc(unsigned long long size) {
    if (size > pool_size[MAX_POOL_ORDER]) {
        return buddy_alloc(size);
    }
    else {
        return dynamic_alloc(size);
    }
}

void *kfree(void *ptr) {
    if ((unsigned long long)ptr % 0x1000 == 0 
        && frame_array[((unsigned long long)ptr - BUDDY_START) >> 12].pool_order == FRAME_NOT_IN_POOL) {
        buddy_free(ptr);
    }
    else {
        dynamic_free(ptr);
    }
}

void memory_reserve(unsigned long long start, unsigned long long end) {
    int start_idx = (align_up((start - 0x1000 + 1), 0x1000) - BUDDY_START) >> 12;
    int end_idx = (align_up(end + 1, 0x1000) - BUDDY_START) >> 12;

    // printf("reserve start_idx : %d\r\n",start_idx);
    // printf("reserve end_idx : %d\r\n",end_idx);

    for (int order = MAX_BUDDY_ORDER; order >= 0; order--) {
        struct list_head *curr;
        list_for_each(curr, &free_list[order]) {
            int page_start = ((frame_t *)curr)->idx;
            int page_end = page_start + (1 << order);
            
            if (start_idx <= page_start && end_idx >= page_end) {
                ((frame_t *)curr)->inused = 1;
                list_del_entry(curr);
                // printf("delete frame idx %d order %d\r\n", ((frame_t *)curr)->idx, ((frame_t *)curr)->order);
            }
            else if (start_idx >= page_end || end_idx <= page_start) {
                continue;
            }
            else {
                list_del_entry(curr);
                list_head_t *temppos = curr -> prev;
                int split_frame_idx = ((frame_t *)curr)->idx + (1 << (order - 1));
                release_redundant_frame(((frame_t *)curr)->idx, order - 1);
                release_redundant_frame(split_frame_idx, order - 1);
                curr = temppos;
            }
        }
    }
}

void test_buddy_alloc() {
    void *addr1 = kmalloc(0x1000);
    printf("malloc address with size 0x1000 : 0x%x\r\n", addr1);
    void *addr2 = kmalloc(0x1000);
    printf("malloc address with size 0x1000 : 0x%x\r\n", addr2);
    void *addr3 = kmalloc(0x1000);
    printf("malloc address with size 0x1000 : 0x%x\r\n", addr3);
    void *addr4 = kmalloc(0x1000);
    printf("malloc address with size 0x1000 : 0x%x\r\n", addr4);
    void *addr5 = kmalloc(0x1000);
    printf("malloc address with size 0x1000 : 0x%x\r\n", addr5);

    kfree(addr2);
    printf("free address with size 0x1000 : 0x%x\r\n", addr2);
    kfree(addr3);
    printf("free address with size 0x1000 : 0x%x\r\n", addr3);
    kfree(addr4);
    printf("free address with size 0x1000 : 0x%x\r\n", addr4);
    kfree(addr1);
    printf("free address with size 0x1000 : 0x%x\r\n", addr1);
}

void test_dynamic_alloc() {
    void *addr1 = kmalloc(96);
    printf("malloc address with size 96 : 0x%x\r\n", addr1);
    show_pool_list(3);
    void *addr2 = kmalloc(96);
    printf("malloc address with size 96 : 0x%x\r\n", addr2);
    show_pool_list(3);

    kfree(addr1);
    printf("free address with size 96 : 0x%x\r\n", addr1);
    show_pool_list(3);
    kfree(addr2);
    printf("free address with size 96 : 0x%x\r\n", addr2);
    show_pool_list(3);

    void *addr3 = kmalloc(96);
    printf("malloc address with size 96 : 0x%x\r\n", addr1);
    show_pool_list(3);
}

void show_free_list(int order) {
    printf("show frames in free_list with order %d :\r\n", order);
    struct list_head *curr;
    list_for_each(curr, &free_list[order])
    {
        printf("frame idx : %d\r\n",((frame_t *)curr)->idx);
    }
}

void show_pool_list(int order) {
    printf("show chunks in pool_list with order %d :\r\n", order);
    struct list_head *curr;
    list_for_each(curr, &pool_list[order]) {
        printf("chunk addr : %x\r\n",curr);
    }
}