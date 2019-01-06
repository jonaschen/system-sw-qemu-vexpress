IMAGE := kernel.elf

CROSS_COMPILE = arm-none-eabi-

export CC = $(CROSS_COMPILE)gcc
export LD = $(CROSS_COMPILE)ld
export OBJDUMP = $(CROSS_COMPILE)objdump


# -ffreestanding: 
#	A freestanding environment is one in which the standard library may not exist, 
#	and program startup may not necessarily be at "main".  
#	The most obvious example is an OS kernel.  This implies -fno-builtin.  
CFLAGS = -Wall -fno-common -O0 -g \
	 -nostdlib -nostartfiles \
	 -ffreestanding \
	 -march=armv7ve

CFLAGS += -Wno-unused-function \
	  -Wno-unused-label

INCLUDE = include
CFLAGS += -I$(INCLUDE)

DIR_DRIVER := driver

export CFLAGS

OBJS = start.o context_switch.o syscall.o
OBJS += main.o irq.o
DRIVER_OBJS = $(DIR_DRIVER)/*.o
OUT := out

all: obj-drivers $(IMAGE)

out-dir:
	mkdir -p $(OUT)

%.o : %.S
	$(CC) -c $(CFLAGS) $< -o $@

obj-drivers:
	$(MAKE) -C $(DIR_DRIVER)

$(IMAGE): kernel.ld $(OBJS) out-dir
	$(LD) $(OBJS) $(DRIVER_OBJS) -T kernel.ld -o $(IMAGE)
	$(OBJDUMP) -d kernel.elf > kernel.list
	mv $(IMAGE) *.o *.list $(OUT)

qemu: $(IMAGE)
	@echo "Press Ctrl-A and then X to exit QEMU"
	@echo
	qemu-system-arm -M vexpress-a9 \
			-m 1024M \
			-nographic \
			-kernel $(OUT)/$(IMAGE)

#debug: -S -s
#arm-none-eabi-gdb
#(gdb) target remote localhost:1234

clean:
	rm -rf $(OUT)
	$(MAKE) clean -C $(DIR_DRIVER)

.PHONY: all qemu clean
