
.PHONY: all clean

INCLUDE = ../include

OBJS += io.o

%.o: %.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -o $@ -c $<

all: $(OBJS)

clean:
	rm -f *.o

