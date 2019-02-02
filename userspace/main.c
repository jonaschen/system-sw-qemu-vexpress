#include <stdint.h>

#include "uart.h"

void main(void)
{
	uint32_t i;

	puts("I am from another world\n");
	puts("But we have connection\n");
	while (1) {
		i = 0U;
		puts("Ha Ha.\n");
		while (i++ < 0xf000000);
	}
}

