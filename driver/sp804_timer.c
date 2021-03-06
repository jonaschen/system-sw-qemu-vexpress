#include "sp804_timer.h"
#include "gic.h"
#include "irq.h"
#include "uart.h"

#define TIMER_ENABLE		(0x1 << 7)
#define TIMER_PERIODIC_MODE	(0x1 << 6)
#define TIMER_INT_ENABLE	(0x1 << 5)

void timer_periodic_setup(uint32_t delay)
{
	uint32_t value;

	/* disable timer */
	write_reg(0, TIMER_1_CONTROL);

	write_reg(delay, TIMER_1_LOAD);

	/* enable timer */
	value = (TIMER_ENABLE | TIMER_PERIODIC_MODE | TIMER_INT_ENABLE);
	write_reg(value, TIMER_1_CONTROL);

}

extern int do_context_switch;
void timer_int_handler(void *arg)
{
	//uint32_t value;

	//value = read_reg(TIMER_1_CONTROL);
	//value &= ~TIMER_INT_ENABLE;
	//write_reg(value, TIMER_1_CONTROL);

	puts("timer irq.\n");
	write_reg(1, TIMER_1_INTCLR);
	do_context_switch = 1;

	return;
}

void timer_delay_awhile(uint32_t delay)
{
	uint32_t value;

	/* disable timer */
	write_reg(0, TIMER_2_CONTROL);

	write_reg(delay, TIMER_2_LOAD);

	/* enable timer */
	value = (TIMER_ENABLE | TIMER_PERIODIC_MODE);
	write_reg(value, TIMER_2_CONTROL);

	while (read_reg(TIMER_2_VALUE) != 0);
}

void setup_timer_irq(void)
{
	struct gic_irq_desc desc;
	uint32_t vector = IRQ_SYS_TIMER_0;

	desc.vector = vector;
	desc.sensitive = GIC_SENS_EDGE;
	desc.priority = 0xE0;

	irq_register_handler(vector, timer_int_handler, 0);
	gic_setup_irq(&desc);
}
