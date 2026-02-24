CC = gcc
CFLAGS = -I../include -Wall -g

test: test_loader
	./test_loader

test_loader: test_loader.c ../loader.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f test_loader
