#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define puts my_puts

int my_puts(const char *str) {
    printf("%s", str);
    return 0;
}

/* Mock ttb addresses */
uint32_t ttb_address = 0x60004000;
uint32_t ttb1_address = 0x60008000;

/* Include the file to test */
#include "../mm.c"

/* Mock task stuff */
struct task_cb mock_current;
struct task_cb *current = &mock_current;


/* Test functions */

int test_allocate_single_page() {
    mm_init();

    uint32_t pfn = allocate_page(1);

    if (pfn != min_pfn) {
        printf("FAIL: test_allocate_single_page: expected %x, got %x\n", min_pfn, pfn);
        return 1;
    }

    /* verify flag was cleared */
    if (page_flags[0] & PAGE_FLAG_AVAILABLE) {
        printf("FAIL: test_allocate_single_page: flag not cleared\n");
        return 1;
    }

    printf("PASS: test_allocate_single_page\n");
    return 0;
}

int test_allocate_multiple_pages() {
    mm_init();

    uint32_t pfn = allocate_page(5);

    if (pfn != min_pfn) {
        printf("FAIL: test_allocate_multiple_pages: expected %x, got %x\n", min_pfn, pfn);
        return 1;
    }

    for (int i = 0; i < 5; i++) {
        if (page_flags[i] & PAGE_FLAG_AVAILABLE) {
            printf("FAIL: test_allocate_multiple_pages: flag not cleared for page %d\n", i);
            return 1;
        }
    }

    if (!(page_flags[5] & PAGE_FLAG_AVAILABLE)) {
        printf("FAIL: test_allocate_multiple_pages: allocated too many pages\n");
        return 1;
    }

    printf("PASS: test_allocate_multiple_pages\n");
    return 0;
}

int test_allocate_out_of_memory() {
    mm_init();

    /* Allocate all pages */
    uint32_t pfn = allocate_page(PHYSICAL_PAGE_FRAMES);

    if (pfn != min_pfn) {
        printf("FAIL: test_allocate_out_of_memory: expected %x, got %x\n", min_pfn, pfn);
        return 1;
    }

    /* Try to allocate one more page */
    pfn = allocate_page(1);
    if (pfn != ~0U) {
        printf("FAIL: test_allocate_out_of_memory: expected ~0U, got %x\n", pfn);
        return 1;
    }

    printf("PASS: test_allocate_out_of_memory\n");
    return 0;
}

int test_allocate_fragmented() {
    mm_init();

    /* Simulate fragmentation: make alternate pages unavailable */
    for (int i = 0; i < PHYSICAL_PAGE_FRAMES; i += 2) {
        page_flags[i] &= ~PAGE_FLAG_AVAILABLE;
    }

    /* We should only be able to allocate 1 page at a time */
    uint32_t pfn1 = allocate_page(1);
    if (pfn1 != min_pfn + 1) {
        printf("FAIL: test_allocate_fragmented: expected %x, got %x\n", min_pfn + 1, pfn1);
        return 1;
    }

    /* Try to allocate 2 pages - should fail */
    uint32_t pfn2 = allocate_page(2);
    if (pfn2 != ~0U) {
        printf("FAIL: test_allocate_fragmented: expected ~0U, got %x\n", pfn2);
        return 1;
    }

    printf("PASS: test_allocate_fragmented\n");
    return 0;
}


int test_allocate_exact_all_memory() {
    mm_init();

    /* Allocate all pages at once */
    uint32_t pfn = allocate_page(PHYSICAL_PAGE_FRAMES);

    if (pfn != min_pfn) {
        printf("FAIL: test_allocate_exact_all_memory: expected %x, got %x\n", min_pfn, pfn);
        return 1;
    }

    /* Verify all flags are cleared */
    for (int i = 0; i < PHYSICAL_PAGE_FRAMES; i++) {
        if (page_flags[i] & PAGE_FLAG_AVAILABLE) {
            printf("FAIL: test_allocate_exact_all_memory: flag not cleared for page %d\n", i);
            return 1;
        }
    }

    /* Verify no more pages can be allocated */
    uint32_t pfn2 = allocate_page(1);
    if (pfn2 != ~0U) {
        printf("FAIL: test_allocate_exact_all_memory: expected ~0U, got %x\n", pfn2);
        return 1;
    }

    printf("PASS: test_allocate_exact_all_memory\n");
    return 0;
}

int test_free_page_basic() {
    mm_init();

    uint32_t pfn = allocate_page(1);
    if (pfn == ~0U) {
        printf("FAIL: test_free_page_basic: allocation failed\n");
        return 1;
    }

    free_page(pfn, 1);

    /* Verify flag was set back to available */
    if (!(page_flags[pfn - min_pfn] & PAGE_FLAG_AVAILABLE)) {
        printf("FAIL: test_free_page_basic: flag not reset after free\n");
        return 1;
    }

    /* Verify we can re-allocate the same page */
    uint32_t pfn2 = allocate_page(1);
    if (pfn2 != pfn) {
        printf("FAIL: test_free_page_basic: re-allocation failed or got different page, expected %x got %x\n", pfn, pfn2);
        return 1;
    }

    printf("PASS: test_free_page_basic\n");
    return 0;
}

int test_free_multiple_pages() {
    mm_init();

    uint32_t pfn = allocate_page(10);
    if (pfn == ~0U) {
        printf("FAIL: test_free_multiple_pages: allocation failed\n");
        return 1;
    }

    free_page(pfn, 10);

    for (int i = 0; i < 10; i++) {
        if (!(page_flags[pfn - min_pfn + i] & PAGE_FLAG_AVAILABLE)) {
            printf("FAIL: test_free_multiple_pages: flag not reset for page %d\n", i);
            return 1;
        }
    }

    /* Re-allocate 10 pages */
    uint32_t pfn2 = allocate_page(10);
    if (pfn2 != pfn) {
        printf("FAIL: test_free_multiple_pages: re-allocation failed or got different page\n");
        return 1;
    }

    printf("PASS: test_free_multiple_pages\n");
    return 0;
}

int test_free_page_out_of_bounds() {
    mm_init();

    /* These should not crash or corrupt other memory */
    free_page(min_pfn - 1, 1);
    free_page(max_pfn, 1);
    free_page(min_pfn, PHYSICAL_PAGE_FRAMES + 1);

    printf("PASS: test_free_page_out_of_bounds (no crash)\n");
    return 0;
}

int test_mm_stress() {
    mm_init();
    uint32_t pfns[100];
    int sizes[100];
    int count = 0;

    printf("Starting mm stress test...\n");

    /* Random-ish allocations */
    for (int i = 0; i < 100; i++) {
        sizes[i] = (i % 10) + 1;
        pfns[i] = allocate_page(sizes[i]);
        if (pfns[i] != ~0U) {
            count++;
        }
    }

    /* Free some of them */
    for (int i = 0; i < 100; i += 2) {
        if (pfns[i] != ~0U) {
            free_page(pfns[i], sizes[i]);
            pfns[i] = ~0U;
            count--;
        }
    }

    /* Allocate again */
    for (int i = 0; i < 50; i++) {
        int size = (i % 5) + 1;
        uint32_t pfn = allocate_page(size);
        if (pfn == ~0U) {
            printf("FAIL: test_mm_stress: allocation failed during stress\n");
            return 1;
        }
    }

    printf("PASS: test_mm_stress\n");
    return 0;
}

int main() {
    int fails = 0;
    printf("Running mm tests...\n");
    fails += test_allocate_single_page();
    fails += test_allocate_multiple_pages();
    fails += test_allocate_out_of_memory();
    fails += test_allocate_fragmented();
    fails += test_allocate_exact_all_memory();
    fails += test_free_page_basic();
    fails += test_free_multiple_pages();
    fails += test_free_page_out_of_bounds();
    fails += test_mm_stress();
    return fails > 0 ? 1 : 0;
}