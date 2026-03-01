#include <stdint.h>

#include "mm.h"
#include "uart.h"
#include "task.h"

extern uint32_t ttb_address;
extern uint32_t ttb1_address;

#define	PAGE_FLAG_AVAILABLE  	(1U << 0)	/* a page available to be allocated */

/*
 * Start: 0x70000000, LENGTH: 0x10000000
 */
#define PHYSICAL_START_FRAME	(0x70000)
#define PHYSICAL_PAGE_FRAMES	(0x10000)
static uint32_t page_flags[PHYSICAL_PAGE_FRAMES];
static uint32_t min_pfn = PHYSICAL_START_FRAME;
static uint32_t max_pfn;

#define VMALLOC_ADDR_START	(0x81000000)	/* 16MB after kernel image */
static void *vmalloc_addr = (void *) VMALLOC_ADDR_START;
static uint32_t	*kernel_pte;			/* VMALLOC_ADDR_START mapping in kernel_pte[0] etc.*/
#define ENTRIES_PER_PTE		(0x100)
#define PAGE_SIZE		(0x1000)
#define PAGE_SHIFT		(12)
#define SECTION_OFFSET		(20)
#define VIRT_TO_PHY(addr)	((uint32_t) (addr) - (0x80000000) + (0x60000000)) /* TODO: replace magic number */
#define PHY_TO_VIRT(addr)	((uint32_t) (addr) - (0x60000000) + (0x80000000) )

/* Each user task is permitted 1MB of virtual space */
#define SIZE_PER_USER_TASK	(0x100000)
static uint32_t	*user_pte;

void switch_user_pte(struct task_cb *tcb)
{
	uint32_t *pgd;
	uint32_t pgd_flags = 0xe5;
	uint32_t pgd_offset;

	if (!tcb || !tcb->pte)
		return;

	pgd_offset = (tcb->vaddr >> SECTION_OFFSET);

	pgd = (uint32_t *) PHY_TO_VIRT(&ttb_address);
	pgd[pgd_offset] = VIRT_TO_PHY(tcb->pte) | pgd_flags;
}

static uint32_t *mm_calculate_pte(uint32_t vaddr)
{
	uint32_t *pte;
	uint32_t vaddr_offset;

	if (vaddr < VMALLOC_ADDR_START) {
		if (!current->pte) {
			current->pte = user_pte;
			user_pte += ENTRIES_PER_PTE;
		}
		pte = current->pte;
	} else {
		vaddr_offset = vaddr - VMALLOC_ADDR_START;
		pte = kernel_pte + (vaddr_offset >> SECTION_OFFSET) * ENTRIES_PER_PTE;
	}

	return pte;
}

void *map_pages(void *addr, uint32_t start_pfn, uint32_t count)
{
/* Calculate the address of PTE table
   check kernel mapping or user mapping
   Calculate the offset of PTE entry
   populate it with flags and physical address
*/
//			return map_pages(i, count);

	uint32_t *pgd;
	uint32_t *pte;
	uint32_t vaddr_base;
	uint32_t vaddr = (uint32_t) addr;
	uint32_t vaddr_offset;
	uint32_t vaddr_end = vaddr + (count << PAGE_SHIFT);
	uint32_t pgd_offset;
	uint32_t pte_offset;
	uint32_t pgd_flags = 0xe5;
	uint32_t pte_flags = 0x436;
	uint32_t pfn = start_pfn;

	if ((uint32_t) vaddr > 0x80000000) {
		pgd = (uint32_t *) PHY_TO_VIRT(&ttb1_address);
		vaddr_base = VMALLOC_ADDR_START;
		vmalloc_addr = (void *) (vaddr + (count << PAGE_SHIFT));	/* TODO: traking vmalloc addresses */
	} else {
		pgd = (uint32_t *) PHY_TO_VIRT(&ttb_address);
		vaddr_base = 0;
	}


	while (vaddr < vaddr_end) {
		pgd_offset = (vaddr >> SECTION_OFFSET);

		vaddr_offset = vaddr - vaddr_base;

		pte = mm_calculate_pte( vaddr);
		pgd[pgd_offset] = VIRT_TO_PHY(pte) | pgd_flags;

		while ((vaddr >> SECTION_OFFSET) == pgd_offset) {
			if (vaddr >= vaddr_end)
				break;

			pte_offset = (vaddr_offset >> PAGE_SHIFT) & (ENTRIES_PER_PTE - 1);
			pte[pte_offset] = (pfn << PAGE_SHIFT) | pte_flags;

			vaddr += PAGE_SIZE;
			pfn++;
		}
	}

	return addr;
}

void *get_page(int count)
{
	uint32_t start;
	void *addr = 0;

	start = allocate_page(count);
	if (start == ~0U)
		return 0;

	addr = map_pages(vmalloc_addr, start, count);

	return addr;
}

uint32_t allocate_page(int count)
{
	uint32_t start, pfn, found;

	for (start = min_pfn; start <= max_pfn - count; start++) {
		found = 1;
		for (pfn = start; pfn < start + count; pfn++) {
			if (!(page_flags[pfn - min_pfn] & PAGE_FLAG_AVAILABLE)) {
				found = 0;
				break;
			}
		}
		if (found) {
			for (pfn = start; pfn < start + count; pfn++)
				page_flags[pfn - min_pfn] &= ~PAGE_FLAG_AVAILABLE;
			return start;
		}
	}

	if (!found) {
		puts("No pages found\n");
		start = ~0U;
	}

	return start;
}

void mm_init(void)
{
	uint32_t pfn;

	/* 0x1000 = 16KB / sizeof(uint32_t) */
	kernel_pte = (uint32_t *) PHY_TO_VIRT(&ttb1_address) + 0x1000;
	user_pte = (uint32_t *) PHY_TO_VIRT(&ttb_address) + 0x1000;

	max_pfn = min_pfn + PHYSICAL_PAGE_FRAMES;

	for (pfn = min_pfn; pfn < max_pfn; pfn++)
		page_flags[pfn - min_pfn] |= PAGE_FLAG_AVAILABLE;
}
