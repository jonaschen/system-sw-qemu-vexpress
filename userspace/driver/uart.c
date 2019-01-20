#include "uart.h"

int putchar(int c)
{
	*((unsigned int *) UART_BASE) = c;
	return 0;
}

int puts(const char *str)
{
	while (*str)
		*((unsigned int *) UART_BASE) = *str++;
	return 0;
}
