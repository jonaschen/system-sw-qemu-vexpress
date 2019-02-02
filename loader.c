#include "uart.h"
#include "elf.h"
#include "task.h"
#include "mm.h"

extern uint32_t _sfirmware;

#define IS_ELF_MAGIC(h)		((h)[0] == ELFMAG0 &&	\
				 (h)[1] == ELFMAG1 &&	\
				 (h)[2] == ELFMAG2 &&	\
				 (h)[3] == ELFMAG3)

static int memcmp(const char *s1, const char *s2, int n)
{
	int i;

	for (i = 0; i < n; i++)
		if (s1[i] != s2[i])
			return -1;

	return 0;
}

void memset(void *addr, uint8_t val, int size)
{
	int i;
	uint8_t *start = (uint8_t *) addr;

	for (i = 0; i < size; i++)
		start[i] = val;
}

void memcpy(void *dst, void *src, int size)
{
	int i;
	uint8_t *from = (uint8_t *) src;
	uint8_t *to = (uint8_t *) dst;

	for (i = 0; i < size; i++)
		to[i] = from[i];
}

int load_user_task(struct task_cb *tcb, uint32_t *task_entry)
{
	char *user_elf= (char *) &_sfirmware;
	Elf32_Ehdr *header = (Elf32_Ehdr *) &_sfirmware;
	Elf32_Shdr *section_headers;
	Elf32_Shdr *text, *data;//, *bss, *strtab;
	char *sh_strtab, *sh_name;
	int i;
	uint32_t start;
	void *vaddr;

	if (IS_ELF_MAGIC(header->e_ident))
		puts("elf magic check ok\n");

	section_headers = (Elf32_Shdr *) &user_elf[header->e_shoff];

	for (i = 0; i < header->e_shnum; i++) {
		if (section_headers[i].sh_type == SHT_STRTAB) {
			sh_strtab = &user_elf[section_headers[i].sh_offset];
			puts("sh strtab found\n");
			break;
		}
	}

	tcb->pte = 0;
	for (i = 0; i < header->e_shnum; i++) {
		sh_name = &sh_strtab[section_headers[i].sh_name];
		if (!memcmp(".text", sh_name, 6)) {
			text = &section_headers[i];
			*task_entry = header->e_entry;
			if (text->sh_addr == header->e_entry)
				puts("text addr check OK\n");
			start = allocate_page(1);
			if (start == 0) {
				puts("invalid pfn\n");
				while (1);
			}
			vaddr = map_pages((void *) text->sh_addr, start, 1);
			tcb->vaddr = (uint32_t) vaddr;
			memcpy(vaddr, &user_elf[text->sh_offset], text->sh_size);
		} else if (!memcmp(".data", sh_name, 6)) {
			data = &section_headers[i];
			start = allocate_page(1);
			if (start == 0) {
				puts("invalid pfn\n");
				while (1);
			}
			vaddr = map_pages((void *) data->sh_addr, start, 1);
			memset(vaddr, 0, 0x1000);
			memcpy(vaddr, &user_elf[data->sh_offset], data->sh_size);
			puts("data section found\n");
		//} else if (!memcmp(".bss", sh_name, 5)) {
		//	bss = &section_headers[i];
		//} else if (!memcmp(".strtab", sh_name, 8)) {
		//	strtab = &section_headers[i];
		}
	}

	return 0;
}
