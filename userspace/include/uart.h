#ifndef __UART_H__
#define __UART_H__

#define UART_BASE	(0x10009000)

#include <stdint.h>

#define write_reg(val, addr)	( *(volatile uint32_t *) (addr) = (val) )
#define read_reg(addr)		( *(volatile uint32_t *) (addr) )


int puts(const char *str);

#endif
