#ifndef __SP804_TIMER_H__
#define __SP804_TIMER_H__

#define TIMER_0_1_BASE		( 0x100e4000 )
#define TIMER_1_LOAD		( ( unsigned long * volatile ) ( TIMER_0_1_BASE + 0x0 ) )	/* Load Register */
#define TIMER_1_VALUE		( ( unsigned long * volatile ) ( TIMER_0_1_BASE + 0x04 ) )	/* Current Value Register */
#define TIMER_1_CONTROL		( ( unsigned long * volatile ) ( TIMER_0_1_BASE + 0x08 ) )	/* Control Register */
#define TIMER_1_INTCLR		( ( unsigned long * volatile ) ( TIMER_0_1_BASE + 0x0C ) )	/* Interrupt Clear Register */
#define TIMER_1_RIS			( ( unsigned long * volatile ) ( TIMER_0_1_BASE + 0x10 ) )	/* Raw Interrupt Status Register */
#define TIMER_1_MIS			( ( unsigned long * volatile ) ( TIMER_0_1_BASE + 0x14 ) )	/* Masked Interrupt Status Register */
#define TIMER_1_BGLOAD		( ( unsigned long * volatile ) ( TIMER_0_1_BASE + 0x18 ) )	/* Background Load Register */

#include <stdint.h>

void timer_delay_awhile(uint32_t delay);

#endif
