#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int mock_puts(const char *s) { return 0; }
uint32_t mock_allocate_page(int count) { return 0; }
void *mock_map_pages(void *addr, uint32_t start_pfn, uint32_t count) { return NULL; }
uint32_t mock_sfirmware = 0;

// To bypass built-in warnings, we can disable them for the test file
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"

// Function under test
extern void memcpy(void *dst, void *src, int size);
extern void memset(void *addr, uint8_t val, int size);

void test_memcpy_basic() {
    char src[] = "Hello World";
    char dst[20] = {0};

    memcpy(dst, src, 12); // "Hello World" + \0

    for (int i=0; i<12; i++) {
        assert(src[i] == dst[i]);
    }
    printf("test_memcpy_basic passed\n");
}

void test_memcpy_empty() {
    char src[] = "Hello World";
    char dst[20] = "Overwrite me";

    memcpy(dst, src, 0);

    char expected[] = "Overwrite me";
    for (int i=0; i<12; i++) {
        assert(expected[i] == dst[i]);
    }
    printf("test_memcpy_empty passed\n");
}

void test_memcpy_partial() {
    char src[] = "Hello World";
    char dst[20] = {0};

    memcpy(dst, src, 5);

    char expected[] = "Hello";
    for (int i=0; i<5; i++) {
        assert(expected[i] == dst[i]);
    }
    assert(dst[5] == 0);
    printf("test_memcpy_partial passed\n");
}

int main() {
    printf("Running memcpy tests...\n");
    test_memcpy_basic();
    test_memcpy_empty();
    test_memcpy_partial();
    printf("All tests passed!\n");
    return 0;
}
