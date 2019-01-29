#include "uart.h"
#include "elf.h"

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

void userspace_loader_test(void)
{
	char *user_elf= (char *) &_sfirmware;
	Elf32_Ehdr *header = (Elf32_Ehdr *) &_sfirmware;
	Elf32_Shdr *section_headers;
	Elf32_Shdr *text, *data, *bss, *strtab;
	char *sh_strtab, *sh_name;
	int i;

	puts("elf parser test\n");

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

	for (i = 0; i < header->e_shnum; i++) {
		sh_name = &sh_strtab[section_headers[i].sh_name];
		if (!memcmp(".text", sh_name, 6)) {
			text = &section_headers[i];
			if (text->sh_addr == header->e_entry)
				puts("text addr check OK\n");
		} else if (!memcmp(".data", sh_name, 6)) {
			data = &section_headers[i];
		} else if (!memcmp(".bss", sh_name, 5)) {
			bss = &section_headers[i];
		} else if (!memcmp(".strtab", sh_name, 8)) {
			strtab = &section_headers[i];
		}
	}
}
