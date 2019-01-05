#include <stdint.h>

#include "irq.h"
#include "uart.h"

struct irq_handler_desc {
	void (*handler)(void *);
	void *arg;
};


struct irq_handler_desc irq_handlers[MAX_IRQ_NUM];

void irq_top_handler(uint32_t irq_num)
{
	struct irq_handler_desc *desc;

	/* assert(irq_num < MAX_IRQ_NUM) */
	if (irq_num > MAX_IRQ_NUM) {
		puts("irq number error.\n");
		while (1);
	}

	desc = &irq_handlers[irq_num];
	if (desc->handler) {
		desc->handler(desc->arg);
	}

	return;
}

int irq_register_handler(uint32_t irq_num, void (*handler)(void *), void *arg)
{
	struct irq_handler_desc *desc;

	if (irq_num > MAX_IRQ_NUM)
		return -1;

	desc = &irq_handlers[irq_num];
	desc->handler = handler;
	desc->arg = arg;

	return 0;
}
