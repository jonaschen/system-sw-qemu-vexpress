.arm

.extern main

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
_swi:			.word undefined_handler
_prefetch_abort:	.word undefined_handler
_data_abort:		.word undefined_handler
_irq:			.word undefined_handler
_fiq:			.word undefined_handler


.text
.align 4
reset_handler: 
	/* set stack pointer, jump to main */
	ldr	r0, =_estack
	mov	sp, r0

	b	main


.align 4
undefined_handler:
	b	.


.end
