#ifndef __REG_H__
#define __REG_H__

#define UART_BASE	(0x10009000)
#define TIMER_0_1_BASE	(0x100e4000)

#define GIC_PERIPHBASE	(0x1e000000)

#include <stdint.h>

#define write_reg(val, addr)	( *(volatile uint32_t *) (addr) = (val) )
#define read_reg(addr)		( *(volatile uint32_t *) (addr) )

#endif
