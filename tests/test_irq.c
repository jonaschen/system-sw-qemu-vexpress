#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

/* Mock `puts` used in `irq.c` */
int mock_puts(const char *s) {
    // printf("%s", s);
    return 0;
}

#include "irq.h"

// Expose the global array from irq.c for testing purposes
extern struct irq_handler_desc {
    void (*handler)(void *);
    void *arg;
} irq_handlers[MAX_IRQ_NUM];

// A dummy handler
int dummy_handler_called = 0;
void dummy_handler(void *arg) {
    dummy_handler_called = 1;
}

void test_irq_register_success(void) {
    // Clear the array first
    memset(irq_handlers, 0, sizeof(irq_handlers));

    int result = irq_register_handler(0, dummy_handler, (void *)0x1234);
    assert(result == 0);
    assert(irq_handlers[0].handler == dummy_handler);
    assert(irq_handlers[0].arg == (void *)0x1234);

    printf("test_irq_register_success passed\n");
}

void test_irq_register_out_of_bounds(void) {
    // MAX_IRQ_NUM should be rejected
    int result = irq_register_handler(MAX_IRQ_NUM, dummy_handler, NULL);
    assert(result == -1);

    result = irq_register_handler(MAX_IRQ_NUM + 1, dummy_handler, NULL);
    assert(result == -1);

    printf("test_irq_register_out_of_bounds passed\n");
}

int main() {
    printf("Running IRQ tests...\n");
    test_irq_register_success();
    test_irq_register_out_of_bounds();
    printf("All tests passed.\n");
    return 0;
}
