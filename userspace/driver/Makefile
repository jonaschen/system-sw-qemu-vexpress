
.PHONY: all clean

INCLUDE = ../include

OBJS += uart.o

%.o: %.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $@ -c $<

all: $(OBJS)

clean:
	rm -f *.o
