#include "dtb.h"
#include "uart.h"
#include "cpio.h"
#include "string.h"
#include "utils.h"

struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

uint32_t u32_little2big(uint32_t data)
{
    char* r = (char*)&data;
    return (r[3]<<0) | (r[2]<<8) | (r[1]<<16) | (r[0]<<24);
}

void fdt_traverse(dtb_callback callback){

    struct fdt_header* header = dtb_addr;
    if(u32_little2big(header->magic) != 0xD00DFEED)
    {
        uart_puts("wrong magic number in fdt_traverse\r\n");
        return;
    }

    uint32_t struct_size = u32_little2big(header->size_dt_struct);
    char* struct_ptr = (char*)((char*)header + u32_little2big(header->off_dt_struct));
    char* strings_ptr = (char*)((char*)header + u32_little2big(header->off_dt_strings));

    char* end = (char*)struct_ptr + struct_size;
    char* pointer = struct_ptr;

    while(pointer < end)
    {
        uint32_t token_type = u32_little2big(*(uint32_t*)pointer);

        pointer += 4;

        if(token_type == FDT_BEGIN_NODE)
        { 
            // printf("FDT_BEGIN_NODE\r\n");
            callback(token_type,pointer,0);
            pointer += strlen(pointer);
            pointer += 4 - (unsigned long long)pointer%4;
        }else if(token_type == FDT_END_NODE)
        {
            // printf("FDT_END_NODE\r\n");
            callback(token_type,0,0);
        }else if(token_type == FDT_PROP)
        {
            // printf("FDT_PROP\r\n");
            uint32_t len = u32_little2big(*(uint32_t*)pointer);
            pointer += 4;
            char* name = (char*)strings_ptr + u32_little2big(*(uint32_t*)pointer);
            pointer += 4;
            callback(token_type,name,pointer);
            pointer += len;
            if((unsigned long long)pointer % 4 !=0)pointer = align_up(pointer,4);
        }else if(token_type == FDT_NOP)
        {
            // printf("FDT_NOP\r\n");
            callback(token_type,0,0);
        }else if(token_type == FDT_END)
        {
            // printf("FDT_END\r\n");
            callback(token_type,0,0);
        }
    }
}

dtb_callback initramfs_callback(uint32_t node_type, char *name, void *value) {
    if(node_type==FDT_PROP && strcmp(name,"linux,initrd-start"))
    {
        cpio_addr = (unsigned long long)u32_little2big(*(uint32_t*)value);
    }
}