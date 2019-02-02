#include "io.h"
#include "uart.h"

void print_int(uint32_t val)
{
	char buf[12] = "0x00000000";
	uint32_t divider = 0xF;
	int i;
	uint8_t cal;

	for (i = 9; i >= 2; i--) {
		if (!val) break;

		cal = (uint8_t) (val & divider);
		if (cal <= 9)
			buf[i] = (char) cal + '0';
		else
			buf[i] = (char) cal - 10 + 'A';

		val >>= 4;
	}

	buf[10] = 0;
	puts(buf);
}

