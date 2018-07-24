.global syscall
syscall:
	push	{lr}

	svc	0
	nop

	pop	{lr}
	bx	lr
