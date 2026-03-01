CC = gcc
CFLAGS = -Wall -g -I../include -DUNIT_TEST

# Disable built-in function mismatch warnings for unit tests since standard C functions are re-implemented in the kernel
CFLAGS += -Wno-builtin-declaration-mismatch

SRCS = test_loader.c ../loader.c
# Put the compiled object files in the current tests directory
OBJS = test_loader.o loader.o
TARGET = test_loader

# Mocks for testing loader.c
CFLAGS += -Dputs=mock_puts -Dallocate_page=mock_allocate_page -Dmap_pages=mock_map_pages -D_sfirmware=mock_sfirmware

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

test_loader.o: test_loader.c
	$(CC) $(CFLAGS) -c -o $@ $<

loader.o: ../loader.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
