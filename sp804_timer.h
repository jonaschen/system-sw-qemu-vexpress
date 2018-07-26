#ifndef __SP804_TIMER_H__
#define __SP804_TIMER_H__

#include "reg.h"

#include <stdint.h>

#define TIMER_1_LOAD		( TIMER_0_1_BASE + 0x0 ) 	/* Load Register */
#define TIMER_1_VALUE		( TIMER_0_1_BASE + 0x04 )	/* Current Value Register */
#define TIMER_1_CONTROL 	( TIMER_0_1_BASE + 0x08 )	/* Control Register */
#define TIMER_1_INTCLR		( TIMER_0_1_BASE + 0x0C )	/* Interrupt Clear Register */
#define TIMER_1_RIS		( TIMER_0_1_BASE + 0x10 )	/* Raw Interrupt Status Register */
#define TIMER_1_MIS		( TIMER_0_1_BASE + 0x14 )	/* Masked Interrupt Status Register */
#define TIMER_1_BGLOAD		( TIMER_0_1_BASE + 0x18 )	/* Background Load Register */


void timer_delay_awhile(uint32_t delay);

#endif
