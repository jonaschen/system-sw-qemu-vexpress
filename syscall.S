.global syscall
syscall:
	svc	0
	nop
	bx	lr
