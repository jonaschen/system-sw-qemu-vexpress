#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Mock environment */
/* Rename functions to avoid conflicts */
#define memset my_memset
#define memcpy my_memcpy
#define memcmp my_memcmp

/* Mock functions required by loader.c */
int puts(const char *str) {
    printf("%s", str);
    return 0;
}

/* Ensure these match declarations in include/mm.h */
uint32_t allocate_page(int count) {
    return 0; // Dummy PFN
}

void *map_pages(void *addr, uint32_t start_pfn, uint32_t count) {
    return addr; // Dummy mapping
}

/* Mock variable */
uint32_t _sfirmware;

/* Include the file to test */
#include "../loader.c"

/* Restore names for clarity (optional, as we test my_*) */
#undef memset
#undef memcpy
#undef memcmp

/* Test functions */
void test_memset_zero() {
    uint8_t buffer[10];
    int i;
    /* Initialize with non-zero */
    for (i = 0; i < 10; i++) buffer[i] = 0xFF;

    my_memset(buffer, 0, 10);

    for (i = 0; i < 10; i++) {
        if (buffer[i] != 0) {
            printf("FAIL: test_memset_zero: index %d is %d\n", i, buffer[i]);
            return;
        }
    }
    printf("PASS: test_memset_zero\n");
}

void test_memset_value() {
    uint8_t buffer[10];
    int i;
    /* Initialize with 0 */
    for (i = 0; i < 10; i++) buffer[i] = 0;

    my_memset(buffer, 0xAA, 10);

    for (i = 0; i < 10; i++) {
        if (buffer[i] != 0xAA) {
            printf("FAIL: test_memset_value: index %d is %02X\n", i, buffer[i]);
            return;
        }
    }
    printf("PASS: test_memset_value\n");
}

void test_memset_partial() {
    uint8_t buffer[10];
    int i;
    /* Initialize with 0xFF */
    for (i = 0; i < 10; i++) buffer[i] = 0xFF;

    my_memset(buffer + 2, 0x55, 5);

    /* Check prefix */
    if (buffer[0] != 0xFF || buffer[1] != 0xFF) {
         printf("FAIL: test_memset_partial: prefix corrupted\n");
         return;
    }
    /* Check modified part */
    for (i = 2; i < 7; i++) {
        if (buffer[i] != 0x55) {
            printf("FAIL: test_memset_partial: index %d is %02X\n", i, buffer[i]);
            return;
        }
    }
    /* Check suffix */
    for (i = 7; i < 10; i++) {
        if (buffer[i] != 0xFF) {
            printf("FAIL: test_memset_partial: suffix corrupted at index %d\n", i);
            return;
        }
    }
    printf("PASS: test_memset_partial\n");
}

int main() {
    printf("Running memset tests...\n");
    test_memset_zero();
    test_memset_value();
    test_memset_partial();
    return 0;
}
