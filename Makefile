RISCV_GNU ?= riscv64-unknown-elf
CC = $(RISCV_GNU)-gcc
LD = $(RISCV_GNU)-ld
OBJCOPY = $(RISCV_GNU)-objcopy
CFLAGS = -mcmodel=medany -ffreestanding -nostdlib -g -Wall
QEMU ?= qemu-system-riscv64
TARGET = kernel.bin
DTB = qemu.dtb

build: clean
	$(CC) $(CFLAGS) -c *.S *.c
	$(LD) -T link.ld -o $(TARGET).elf *.o
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET)

# Generate device tree blob from QEMU
$(DTB):
	$(QEMU) -M virt -m 128M -machine virt,dumpdtb=$(DTB) -nographic > /dev/null 2>&1 || true

run: build $(DTB)
	$(QEMU) -M virt -m 128M -kernel $(TARGET) -dtb $(DTB) -display none -serial stdio

debug: build $(DTB)
	$(QEMU) -M virt -m 128M -kernel $(TARGET) -dtb $(DTB) -display none -serial stdio -s -S

clean:
	rm -f $(TARGET) $(TARGET).elf *.o $(DTB)