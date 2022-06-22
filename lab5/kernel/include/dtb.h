#ifndef DTB_H
#define DTB_H

#define uint32_t unsigned int

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

unsigned long dtb_addr;
typedef void (*dtb_callback)(uint32_t node_type, char *name, void *value);

void fdt_traverse(dtb_callback initramfs_callback);
dtb_callback initramfs_callback(uint32_t node_type, char *name, void *value);

#endif