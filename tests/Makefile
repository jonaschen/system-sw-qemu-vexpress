CC = gcc
CFLAGS = -Wall -I../include -O0 -g

# Test executables
TEST_IRQ = test_irq
TEST_LOADER = test_loader
TEST_MM = test_mm

# Source files for each test
TEST_IRQ_SRCS = test_irq.c ../irq.c
TEST_LOADER_SRCS = test_loader.c 
TEST_MM_SRCS = test_mm.c 

TARGETS = $(TEST_IRQ) $(TEST_LOADER) $(TEST_MM)

all: $(TARGETS) run

$(TEST_MM): $(TEST_MM_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# test_irq needs mock_puts defined via compiler flag
$(TEST_IRQ): $(TEST_IRQ_SRCS)
	$(CC) $(CFLAGS) -Dputs=mock_puts -o $@ $^

# test_loader defines its own puts mock in the source file
$(TEST_LOADER): $(TEST_LOADER_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

run: $(TARGETS)
	@echo "Running IRQ tests..."
	./$(TEST_IRQ)
	@echo ""
	@echo "Running loader tests..."
	./$(TEST_LOADER)
	@echo ""
	@echo "Running mm tests..."
	./$(TEST_MM)
	@echo ""
	@echo "All tests completed successfully!"


clean:
	rm -f $(TARGETS)

.PHONY: all run clean
