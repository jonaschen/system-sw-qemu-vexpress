#ifndef __MM_H__
#define __MM_H__

#include <stdint.h>
#include "task.h"

void mm_init(void);
void *get_page(int count);
uint32_t allocate_page(int count);

/**
 * @brief Reclaim physical memory pages.
 *
 * @param pfn The starting Page Frame Number (PFN) to free.
 * @param count The number of contiguous pages to free.
 */
void free_page(uint32_t pfn, int count);

void *map_pages(void *addr, uint32_t start_pfn, uint32_t count);
void switch_user_pte(struct task_cb *tcb);

#endif
