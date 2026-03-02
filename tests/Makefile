CC = gcc
CFLAGS = -Wall -I../include -O0 -g

# Test executables
TEST_IRQ = test_irq
TEST_LOADER = test_loader
TEST_MEMSET = test_memset

# Source files for each test
TEST_IRQ_SRCS = test_irq.c ../irq.c
TEST_LOADER_SRCS = test_loader.c 
TEST_MEMSET_SRCS = test_memset.c ../loader.c 

TARGETS = $(TEST_IRQ) $(TEST_LOADER) $(TEST_MEMSET)

all: $(TARGETS) run

# test_irq needs mock_puts defined via compiler flag
$(TEST_IRQ): $(TEST_IRQ_SRCS)
	$(CC) $(CFLAGS) -Dputs=mock_puts -o $@ $^

# test_loader defines its own puts mock in the source file
$(TEST_LOADER): $(TEST_LOADER_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_MEMSET): $(TEST_MEMSET_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

run: $(TARGETS)
	@echo "Running IRQ tests..."
	./$(TEST_IRQ)
	@echo ""
	@echo "Running loader tests..."
	./$(TEST_LOADER)
	@echo ""
	@echo "Running memset tests..."
	./$(TEST_MEMSET)
	@echo ""
	@echo "All tests completed successfully!"

clean:
	rm -f $(TARGETS)

.PHONY: all run clean
