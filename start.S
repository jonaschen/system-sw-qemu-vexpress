#include "processor.h"

.arm

.extern init
.extern irq_top_handler
.extern do_context_switch
.extern do_system_service

.section .vectors
_vector_table:
	b	reset_handler
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
	bl	clean_ttb_dir
	ldr	r1, =ttb1_address
	bl	clean_ttb_dir

	orr	r0, r0, #0b10
	/*
 	 * Normal Memory Mapping
	 */
	bic	r0, r0, #0b1100		// clear CB bits
	orr	r0, r0, #0b0100		// inner write-back, write allocate
	bic	r0, r0, #(0b111 << 12)	// clear TEX bits
	orr	r0, r0, #(0b101 << 12)	// set TEX as write-back, write allocate
	orr	r0, r0, #(0x1 << 16)	// shareable

	ldr	r1, =ttb_address
	ldr	r2, =normal_memory_base
	ldr	r3, =normal_memory_base
	ldr	r4, =normal_memory_size
	bl	set_l1_dir

	ldr	r1, =ttb1_address
	ldr	r2, =kernel_image_base
	ldr	r3, =KERNEL_OFFSET
	ldr	r4, =kernel_image_size
	bl	set_l1_dir

	/*
 	 * Device Memory Mapping (mapping as strongly-ordered)
	 * TEX = b'000, C = 0, B= 0
	 */
	bic	r0, r0, #0b1100		// clear CB bits
	bic	r0, r0, #(0b111 << 12)	// clear TEX bits

	ldr	r1, =ttb_address
	ldr	r2, =peripheral_device_base
	ldr	r3, =peripheral_device_base
	ldr	r4, =peripheral_device_size
	bl	set_l1_dir

	/* Initialize MMU */
	mov	r1, #0x0
	orr	r1, r1, #0b001 		/* Setting T0SZ for switching translation tables */
	mcr	p15, 0, r1, c2, c0, 2	// Write Translation Table Base Control Register
	ldr	r1, =ttb_address
	mcr	p15, 0, r1, c2, c0, 0	// Write Translation Table base Register 0
	ldr	r1, =ttb1_address
	mcr	p15, 0, r1, c2, c0, 1	// Write TTBR1
	ldr	r1, =0x55555555
	mcr	p15, 0, r1, c3, c0, 0	// Write Domain Access Control Register

	/* Enable MMU */
	mrc	p15, 0, r1, c1, c0, 0
	orr	r1, r1, #0x1
	mcr	p15, 0, r1, c1, c0, 0

	ISB
	DSB

	ldr	pc, =mmu_enabled
mmu_enabled:
	b	init

	b	.

.align 4
svc_handler:
	/* save user's context
	 * r0-r12, lr, cpsr
	 */
	srsdb	sp!, #SYSTEM_MODE
	cps	#SYSTEM_MODE
	push	{r1-r12, lr}

	cmp	r7, #100
	bne	do_service

	/* pass the stack pointer to OS through r0*/
	push	{r0}
	mov	r0, sp

	cps	#SVC_MODE
	pop	{r4-r12, lr}
	bx	lr

do_service:
	mov 	r3, r2
	mov 	r2, r1
	mov 	r1, r0
	mov	r0, r7
	bl	do_system_service
	pop	{r1-r12, lr}
	rfeia	sp!

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

	/* jump to top irq handler */
	bl	irq_top_handler

	/* pass the stack pointer to OS through r0*/
	ldr	r4, =do_context_switch
	ldr	r5, [r4]
	cmp	r5, #0
	bne	back_to_kernel
	pop	{r0-r12, lr}
	rfeia	sp!

back_to_kernel:
	mov	r0, sp
	mov	r5, #0
	str	r5, [r4]
	cps	#SVC_MODE
	pop	{r4-r12, lr}
	bx	lr

.align 4
set_l1_dir:
	/* r0:flags, r1:ttb_addr, r2:vaddr, r3:paddr, r4:length
	   r7: temp
	 */
	lsr	r2, r2, #20
	lsr	r3, r3, #20
	lsr	r4, r4, #20
	add	r4, r3, r4
et_l1_dir_loop:
	orr	r7, r0, r2, LSL #20
	str	r7, [r1, r3, LSL #2]
	add	r3, r3, #1
	add	r2, r2, #1
	cmp	r3, r4
	bne	et_l1_dir_loop

	bx	lr

.align 4
clean_ttb_dir:
	/* r0:flags, r1:ttb_addr */
	ldr	r3, =4096
clean_ttb_dir_loop:
	orr	r2, r0, r3, LSL #20
	str	r2, [r1, r3, LSL #2]
	subs	r3, r3, #1
	bne	clean_ttb_dir_loop

	bx	lr

.end
