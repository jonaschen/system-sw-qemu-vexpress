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
	 -march=armv7ve \
	 -fPIC

CFLAGS += -Wno-unused-function \
	  -Wno-unused-label

LDFLAGS = -e 0x60000000	# TODO: link change with ld script

INCLUDE = include
CFLAGS += -I$(INCLUDE)

DRIVER_DIR := driver
USER_DIR := userspace
LIB_DIR := lib

export CFLAGS

OBJS = start.o context_switch.o syscall.o
OBJS += main.o irq.o mm.o loader.o
DRIVER_OBJS = $(DRIVER_DIR)/*.o
LIB_OBJS = $(LIB_DIR)/*.o
OUT := out

all: obj-drivers obj-user obj-lib $(IMAGE)

out-dir:
	mkdir -p $(OUT)

%.o : %.S
	$(CC) -c $(CFLAGS) $< -o $@

obj-drivers:
	$(MAKE) -C $(DRIVER_DIR)

obj-user:
	$(MAKE) -C $(USER_DIR)

obj-lib:
	$(MAKE) -C $(LIB_DIR)

$(IMAGE): kernel.ld $(OBJS) out-dir
	$(LD) $(OBJS) $(DRIVER_OBJS) $(LIB_OBJS) $(LDFLAGS) -T kernel.ld -o $(IMAGE) --print-map > kernel.map
	$(OBJDUMP) -d kernel.elf > kernel.list
	mv $(IMAGE) *.o *.list *.map $(OUT)
	dd if=$(OUT)/kernel.elf of=$(OUT)/test.elf
	dd if=$(USER_DIR)/out/hello.elf of=$(OUT)/test.elf conv=notrunc bs=1 seek=`echo "ibase=16; 10100" | bc`

# convert 0x10100 to decimal: `echo "ibase=16; 10100" | bc`

qemu: $(IMAGE)
	@echo "Press Ctrl-A and then X to exit QEMU"
	@echo
	qemu-system-arm -M vexpress-a9 \
			-m 1024M \
			-nographic \
			-kernel $(OUT)/test.elf

#			-kernel $(OUT)/$(IMAGE)
#debug: -S -s
#arm-none-eabi-gdb
#(gdb) target remote localhost:1234
#(gdb) restore user.elf binary 0x0

clean:
	rm -rf $(OUT)
	$(MAKE) clean -C $(DRIVER_DIR)
	$(MAKE) clean -C $(USER_DIR)
	$(MAKE) clean -C $(LIB_DIR)

.PHONY: all qemu clean test


# Test target for unit tests
test: out-dir
	gcc -I$(INCLUDE) -I. -g -Wall tests/test_loader.c -o $(OUT)/test_loader
	@echo "Running tests..."
	@$(OUT)/test_loader
