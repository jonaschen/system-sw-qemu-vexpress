#ifndef __IRQ_H__
#define __IRQ_H__

/*
 * http://infocenter.arm.com/help/topic/com.arm.doc.dui0448i/DUI0448I_v2p_ca9_trm.pdf page.32
 */

#define IRQ0_UART		(32 + 5) /* from Motherboard */
#define IRQ_SYS_TIMER_0		(80)
#define IRQ_UART		(84)

#define MAX_IRQ_NUM		(100)

int irq_register_handler(uint32_t irq_num, void (*handler)(void *), void *arg);
#endif
