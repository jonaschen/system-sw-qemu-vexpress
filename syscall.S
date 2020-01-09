#define ENTRY(name)	\
	.globl name;    \
	.align 4;       \
	name:

#define END(name) .size name, .- name

#define ENDPROC(name)		\
	.type name, % function;	\
	END(name)


ENTRY(do_syscall0)
	mov	r7, r0
	svc	0
	nop
	bx	lr
ENDPROC(do_syscall0)

ENTRY(do_syscall1)
	mov	r7, r1
	svc	0
	nop
	bx	lr
ENDPROC(do_syscall1)

ENTRY(do_syscall2)
	mov	r7, r2
	svc	0
	nop
	bx	lr
ENDPROC(do_syscall2)

ENTRY(do_syscall3)
	mov	r7, r3
	svc	0
	nop
	bx	lr
ENDPROC(do_syscall3)
