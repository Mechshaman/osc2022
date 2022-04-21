CFLAGS = -Wall -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector -Iinclude

# -Wall ： enable all warnings
# -ffreestanding  ： freestanding environment
# -nostdinc : Do not search the standard system directories for header files
# -nostdlib ： Do not use the standard system startup files or libraries when linking.
# -nostartfiles ： Do not use the standard system startup files when linking.
# -fno-stack-protector ： 禁用堆疊保護 不然會有 undefined reference to `__stack_chk_guard'
# -I ： Add the directory dir to the list of directories to be searched for header files during preprocessing.

SRC_DIR = src
BUILD_DIR = build

.PHONY: all clean run debug

all: clean bootloader.img

OLD_OBJ_FILES = $(wildcard $(BUILD_DIR)/*.o)

clean:
	rm -f $(OLD_OBJ_FILES) $(BUILD_DIR)/bootloader.elf bootloader.img

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	aarch64-elf-gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	aarch64-elf-gcc $(CFLAGS) -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
S_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(S_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

bootloader.img: linker.ld $(OBJ_FILES)
	aarch64-elf-ld -nostdlib -nostartfiles $(OBJ_FILES) -T linker.ld -o $(BUILD_DIR)/bootloader.elf
	aarch64-elf-objcopy -O binary $(BUILD_DIR)/bootloader.elf bootloader.img

run:
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -display none -serial null -serial stdio -dtb bcm2710-rpi-3-b-plus.dtb

pty:
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -display none -serial null -serial pty -dtb bcm2710-rpi-3-b-plus.dtb

debug:
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -display none -S -s -dtb bcm2710-rpi-3-b-plus.dtb

ptydebug:
	qemu-system-aarch64 -M raspi3 -kernel bootloader.img -display none -serial null -serial pty -s -S -dtb bcm2710-rpi-3-b-plus.dtb

raspi:
	sudo screen /dev/ttyUSB0 115200
