#include "sp804_timer.h"


#define TIMER_ENABLE		(0x1 << 7)
#define TIMER_PERIODIC_MODE	(0x1 << 6)

void timer_delay_awhile(uint32_t delay)
{

	/* disable timer */
	*TIMER_1_CONTROL = 0;

	*TIMER_1_LOAD = delay;

	/* enable timer */
	*TIMER_1_CONTROL = (TIMER_ENABLE | TIMER_PERIODIC_MODE);

	while (*TIMER_1_VALUE != 0);
}
