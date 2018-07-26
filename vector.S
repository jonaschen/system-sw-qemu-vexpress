#include "processor.h"

.arm

.extern init

.section .vectors
_vector_table:
	ldr	pc, _boot
	ldr	pc, _undef
	ldr	pc, _swi
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	nop
	ldr	pc, _irq
	ldr	pc, _fiq


_boot:			.word reset_handler
_undef:			.word undefined_handler
_swi:			.word svc_handler
_prefetch_abort:	.word undefined_handler
_data_abort:		.word undefined_handler
_irq:			.word irq_handler
_fiq:			.word undefined_handler


.text
.align 4
reset_handler:
	/* set stack pointer, jump to main */
	ldr	r0, =_estack
	mov	sp, r0

	/* Vector base address */
	ldr	r0, =__vector_base
	mcr 	p15, 0, r0, c12, c0, 0

	b	init

	b	.

.align 4
svc_handler:
	/* save user's context
	 * r0-r12, lr, cpsr
	 */
	srsdb	sp!, #SYSTEM_MODE
	cps	#SYSTEM_MODE
	push	{r0-r12, lr}

	/* pass the stack pointer to OS through r0*/
	mov	r0, sp

	cps	#SVC_MODE
	pop	{r4-r12, lr}
	bx	lr

.align 4
undefined_handler:
	b	.

.align 4
irq_handler:
	ldr	r1, =0x1e00010c
	ldr	r0, [r1]
	b	.

.end
