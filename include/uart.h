#ifndef __UART_H__
#define __UART_H__

#include "reg.h"

/*
 * http://infocenter.arm.com/help/topic/com.arm.doc.ddi0183f/DDI0183.pdf
 */

#define	UART_DR		(UART_BASE + 0x00UL)
#define	UART_IMSC	(UART_BASE + 0x38UL)
#define UART_MIS	(UART_BASE + 0x40UL)
#define UART_ICR	(UART_BASE + 0x44UL)

/* UART_IMSC */
#define UART_IMSC_RXIM	(1 << 0x4)
/* */

int puts(const char *str);
int putchar(int c);
void uart_init(void);
char uart_buf_getchar(void);

#endif
