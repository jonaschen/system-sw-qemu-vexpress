CC = gcc
CFLAGS = -I../include -Wall -g

test: test_mm
	./test_mm

test_mm: test_mm.c ../mm.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f test_mm
