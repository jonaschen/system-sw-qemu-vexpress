
.PHONY: all clean

INCLUDE = ../include

OBJS += gic.o sp804_timer.o uart.o

%.o: %.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $@ -c $<

all: $(OBJS)

clean:
	rm -f *.o
