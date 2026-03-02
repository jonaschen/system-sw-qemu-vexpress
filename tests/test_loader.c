#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


/* Mock environment */
/* Rename functions to avoid conflicts */
#define memset my_memset
#define memcpy my_memcpy
#define memcmp my_memcmp
#define puts loader_puts

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
#undef puts

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

void test_memcmp() {
    printf("Running memcmp tests...\n");

    // Exact match
    assert(my_memcmp("hello", "hello", 5) == 0);
    assert(my_memcmp("", "", 0) == 0);

    // Mismatch at start
    assert(my_memcmp("hello", "yello", 5) == -1);

    // Mismatch at end
    assert(my_memcmp("hello", "hella", 5) == -1);

    // Mismatch in middle
    assert(my_memcmp("hello", "heXlo", 5) == -1);

    // Partial match (prefix)
    assert(my_memcmp("hello", "help", 3) == 0);
    assert(my_memcmp("hello", "help", 4) == -1);

    // Testing unaligned and different length
    assert(my_memcmp("abc", "abd", 3) == -1);
    assert(my_memcmp("abc", "abc", 3) == 0);

    printf("All memcmp tests passed!\n");
}

void test_memcpy_basic() {
    char src[] = "Hello World";
    char dst[20] = {0};

    my_memcpy(dst, src, 12); // "Hello World" + \0

    for (int i=0; i<12; i++) {
        assert(src[i] == dst[i]);
    }
    printf("test_memcpy_basic passed\n");
}

void test_memcpy_empty() {
    char src[] = "Hello World";
    char dst[20] = "Overwrite me";

    my_memcpy(dst, src, 0);

    char expected[] = "Overwrite me";
    for (int i=0; i<12; i++) {
        assert(expected[i] == dst[i]);
    }
    printf("test_memcpy_empty passed\n");
}

void test_memcpy_partial() {
    char src[] = "Hello World";
    char dst[20] = {0};

    my_memcpy(dst, src, 5);

    char expected[] = "Hello";
    for (int i=0; i<5; i++) {
        assert(expected[i] == dst[i]);
    }
    assert(dst[5] == 0);
    printf("test_memcpy_partial passed\n");
}

int main() {
    printf("Running memset tests...\n");
    test_memcpy_basic();
    test_memcpy_empty();
    test_memcpy_partial();
    test_memset_zero();
    test_memset_value();
    test_memset_partial();
    test_memcmp();

    return 0;
}
