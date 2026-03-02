#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

// Mock functions to satisfy linker
uint32_t _sfirmware = 0;
int my_puts(const char *s) { return 0; }
uint32_t my_allocate_page(int n) { return 0; }
void *my_map_pages(void *addr, uint32_t start_pfn, uint32_t count) { return NULL; }

extern void my_memset(void *addr, uint8_t val, int size);

void test_basic_memset() {
    uint8_t buffer[100];

    // Initialize buffer to a known state
    for (int i = 0; i < 100; i++) buffer[i] = 0;

    // Set first 10 bytes to 0xAA
    my_memset(buffer, 0xAA, 10);

    // Verify
    for (int i = 0; i < 10; i++) {
        if (buffer[i] != 0xAA) {
            printf("test_basic_memset FAILED at index %d: expected 0xAA, got 0x%02X\n", i, buffer[i]);
            exit(1);
        }
    }
    for (int i = 10; i < 100; i++) {
        if (buffer[i] != 0) {
            printf("test_basic_memset FAILED at index %d: expected 0x00, got 0x%02X\n", i, buffer[i]);
            exit(1);
        }
    }
    printf("test_basic_memset PASSED\n");
}

void test_memset_zero_size() {
    uint8_t buffer[10];
    for (int i = 0; i < 10; i++) buffer[i] = 0xFF;

    my_memset(buffer, 0x00, 0);

    for (int i = 0; i < 10; i++) {
        if (buffer[i] != 0xFF) {
            printf("test_memset_zero_size FAILED at index %d: expected 0xFF, got 0x%02X\n", i, buffer[i]);
            exit(1);
        }
    }
    printf("test_memset_zero_size PASSED\n");
}

void test_memset_entire_buffer() {
    uint8_t buffer[50];
    for (int i = 0; i < 50; i++) buffer[i] = 0;

    my_memset(buffer, 0x55, 50);

    for (int i = 0; i < 50; i++) {
        if (buffer[i] != 0x55) {
            printf("test_memset_entire_buffer FAILED at index %d: expected 0x55, got 0x%02X\n", i, buffer[i]);
            exit(1);
        }
    }
    printf("test_memset_entire_buffer PASSED\n");
}

int main() {
    printf("Running memset tests...\n");
    test_basic_memset();
    test_memset_zero_size();
    test_memset_entire_buffer();
    printf("All memset tests PASSED!\n");
    return 0;
}
