CC = gcc
CFLAGS = -I../include -Dmemset=my_memset -Dmemcmp=my_memcmp -Dmemcpy=my_memcpy -Dputs=my_puts -Dallocate_page=my_allocate_page -Dmap_pages=my_map_pages -fno-builtin

all: test

test: test_memset
	./test_memset

test_memset: test_memset.c ../loader.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f test_memset

.PHONY: all test clean
