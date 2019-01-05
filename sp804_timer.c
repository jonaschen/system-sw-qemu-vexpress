#include "sp804_timer.h"
#include "gic.h"
#include "irq.h"
#include "io.h"

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

void timer_clear_int(void *arg)
{
	//uint32_t value;

	//value = read_reg(TIMER_1_CONTROL);
	//value &= ~TIMER_INT_ENABLE;
	//write_reg(value, TIMER_1_CONTROL);

	puts("timer irq.\n");
	write_reg(1, TIMER_1_INTCLR);

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
	uint32_t vector = 32 + 48; /* number of PPI + timer-SPI-offset */

	desc.vector = vector;
	desc.sensitive = GIC_SENS_EDGE;
	desc.priority = 0xE0;

	irq_register_handler(vector, timer_clear_int, 0);
	gic_setup_irq(&desc);
}
