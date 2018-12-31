#include "processor.h"

.arm

.extern init
.extern timer_clear_int

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
_data_abort:		.word abort_handler
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

	/* Create identity 1M-section mapping
	 * bit[31:20]: Section Base Address
	 * bit[19:18]: SBZ, bit[17]: nG, bit[16]:S, bit[15]:APX
	 * bit[14:12]: TEX, bit[11:10]: AP, bit[9]: P
	 * bit[8:5]: Domain, bit[4]:XN, bit[3]:C, bit[2]:B, bit[1:0]: b'10
	 */
	/*
	 * APX = 0, AP = b'11 --> Full access
	 */
	ldr	r0, =0b00000000000000000000110111100000
	ldr	r1, =ttb_address
	ldr	r3, =4096
clean_dir:
	orr	r2, r0, r3, LSL #20
	str	r2, [r1, r3, LSL #2]
	subs	r3, r3, #1
	bne	clean_dir

	orr	r0, r0, #0b10
	/*
 	 * Normal Memory Mapping
	 */
	bic	r0, r0, #0b1100		// clear CB bits
	orr	r0, r0, #0b0100		// inner write-back, write allocate
	bic	r0, r0, #(0b111 << 12)	// clear TEX bits
	orr	r0, r0, #(0b101 << 12)	// set TEX as write-back, write allocate
	orr	r0, r0, #(0x1 << 16)	// shareable

	ldr	r5, =normal_memory_base
	lsr	r3, r5, #20
	ldr	r5, =normal_memory_size
	lsr	r4, r5, #20
	add	r4, r3, r4
	ldr	r1, =ttb_address
set_memory_dir:
	orr	r2, r0, r3, LSL #20
	str	r2, [r1, r3, LSL #2]
	adds	r3, r3, #1
	cmp	r3, r4
	bne	set_memory_dir

	/*
 	 * Device Memory Mapping (mapping as strongly-ordered)
	 * TEX = b'000, C = 0, B= 0
	 */
	bic	r0, r0, #0b1100		// clear CB bits
	bic	r0, r0, #(0b111 << 12)	// clear TEX bits

	ldr	r5, =peripheral_device_base
	lsr	r3, r5, #20
	ldr	r5, =peripheral_device_size
	lsr	r4, r5, #20
	add	r4, r3, r4
	ldr	r1, =ttb_address
set_device_dir:
	orr	r2, r0, r3, LSL #20
	str	r2, [r1, r3, LSL #2]
	adds	r3, r3, #1
	cmp	r3, r4
	bne	set_device_dir

	/* Initialize MMU */
	mov	r1, #0x0
	mcr	p15, 0, r1, c2, c0, 2	// Write Translation Table Base Control Register
	ldr	r1, =ttb_address
	mcr	p15, 0, r1, c2, c0, 0	// Write Translation Table base Register 0
	ldr	r1, =0x55555555
	mcr	p15, 0, r1, c3, c0, 0	// Write Domain Access Control Register

	/* Enable MMU */
	mrc	p15, 0, r1, c1, c0, 0
	orr	r1, r1, #0x1
	mcr	p15, 0, r1, c1, c0, 0

	ISB
	DSB

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
abort_handler:
	b	.

.align 4
irq_handler:
	/* save user's context
	 * r0-r12, lr, cpsr
	 */
	sub	lr, #0x4
	srsdb	sp!, #SYSTEM_MODE
	cps	#SYSTEM_MODE
	push	{r0-r12, lr}

	/* ack irq by reading GICC_IAR*/
	ldr	r1, =0x1e00010c
	ldr	r0, [r1]

	/* end irq by writing GICC_EOIR */
	ldr	r1, =0x1e000110
	str	r0, [r1]

	/* handling timer isr */
	mov	r1, #0x50
	cmp	r0, r1
	bleq	timer_clear_int

	/* pass the stack pointer to OS through r0*/
	mov	r0, sp

	cps	#SVC_MODE
	pop	{r4-r12, lr}
	bx	lr



.end
