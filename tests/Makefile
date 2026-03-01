CC = gcc
CFLAGS = -Wall -I../include -O0 -g

# Mock puts so we don't need a real UART or real kernel
CFLAGS += -Dputs=mock_puts

SRCS = test_irq.c ../irq.c
TARGET = test_irq

all: $(TARGET) run

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
