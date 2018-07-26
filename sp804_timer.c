#include "sp804_timer.h"


#define TIMER_ENABLE		(0x1 << 7)
#define TIMER_PERIODIC_MODE	(0x1 << 6)
#define TIMER_INT_ENABLE	(0x1 << 5)

void timer_delay_awhile(uint32_t delay)
{
	uint32_t value;

	/* disable timer */
	write_reg(0, TIMER_1_CONTROL);

	write_reg(delay, TIMER_1_LOAD);

	/* enable timer */
	value = (TIMER_ENABLE | TIMER_PERIODIC_MODE | TIMER_INT_ENABLE);
	write_reg(value, TIMER_1_CONTROL);

	while (read_reg(TIMER_1_VALUE) != 0);
}
