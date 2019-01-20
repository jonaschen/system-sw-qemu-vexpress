IMAGE := hello.elf

CROSS_COMPILE = arm-none-eabi-

export CC = $(CROSS_COMPILE)gcc
export LD = $(CROSS_COMPILE)ld
export OBJDUMP = $(CROSS_COMPILE)objdump


CFLAGS = -Wall -fno-common -O0 \
	 -nostdlib -nostartfiles \
	 -ffreestanding \
	 -march=armv7ve \
	 -fPIC

CFLAGS += -Wno-unused-function \
	  -Wno-unused-label

INCLUDE = include
CFLAGS += -I$(INCLUDE)

DIR_DRIVER := driver

export CFLAGS

OBJS = main.o
DRIVER_OBJS = $(DIR_DRIVER)/*.o
OUT := out

all: obj-drivers $(IMAGE)

out-dir:
	mkdir -p $(OUT)

%.o : %.S
	$(CC) -c $(CFLAGS) $< -o $@

obj-drivers:
	$(MAKE) -C $(DIR_DRIVER)

$(IMAGE): hello.ld $(OBJS) out-dir
	$(LD) $(OBJS) $(DRIVER_OBJS) $(LDFLAGS) -T hello.ld -o $(IMAGE) --print-map > hello.map --nmagic
	$(OBJDUMP) -d hello.elf > hello.list
	mv $(IMAGE) *.o *.list *.map $(OUT)


clean:
	rm -rf $(OUT)
	$(MAKE) clean -C $(DIR_DRIVER)

.PHONY: all clean
