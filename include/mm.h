#ifndef __MM_H__
#define __MM_H__

#include <stdint.h>
#include "task.h"

void mm_init(void);
void *get_page(int count);
uint32_t allocate_page(int count);
void *map_pages(void *addr, uint32_t start_pfn, uint32_t count);
void switch_user_pte(struct task_cb *tcb);

#endif
