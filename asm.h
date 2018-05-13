#ifndef __ASM_H__
#define __ASM_H__


/*
 * activate an user task
 * return value: current stack pointer
 */
unsigned int *activate(unsigned int *stack);

/*
 * give control back to the Operating System
 */
void syscall(void);

#endif
