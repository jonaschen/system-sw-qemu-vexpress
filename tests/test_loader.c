#include <stdint.h>
#include <stdio.h>
#include <assert.h>

uint32_t _sfirmware = 0;
// Use standard library for I/O in the mock
int loader_puts(const char *s) {
    return fputs(s, stdout) != EOF && fputc('\n', stdout) != EOF ? 0 : -1;
}

uint32_t allocate_page(int n) { return 0; }
void *map_pages(void *addr, uint32_t start_pfn, uint32_t count) { return 0; }

// Rename functions to avoid conflicts
#define memcmp loader_memcmp
#define memset loader_memset
#define memcpy loader_memcpy
#define puts loader_puts

#include "../loader.c"

#undef memcmp
#undef memset
#undef memcpy
#undef puts

void test_memcmp() {
    printf("Running memcmp tests...\n");

    // Exact match
    assert(loader_memcmp("hello", "hello", 5) == 0);
    assert(loader_memcmp("", "", 0) == 0);

    // Mismatch at start
    assert(loader_memcmp("hello", "yello", 5) == -1);

    // Mismatch at end
    assert(loader_memcmp("hello", "hella", 5) == -1);

    // Mismatch in middle
    assert(loader_memcmp("hello", "heXlo", 5) == -1);

    // Partial match (prefix)
    assert(loader_memcmp("hello", "help", 3) == 0);
    assert(loader_memcmp("hello", "help", 4) == -1);

    // Testing unaligned and different length
    assert(loader_memcmp("abc", "abd", 3) == -1);
    assert(loader_memcmp("abc", "abc", 3) == 0);

    printf("All memcmp tests passed!\n");
}

int main() {
    printf("Starting tests...\n");
    test_memcmp();
    return 0;
}
