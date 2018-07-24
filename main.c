#include <stdint.h>

#include "reg.h"
#include "asm.h"
#include "processor.h"
#include "sp804_timer.h"


int puts(const char *str)
{
	while (*str)
		*((unsigned int *) UART_BASE) = *str++;
	return 0;
}

/* start address for the initialization values of the .data section.
defined in linker script */
extern uint32_t _sidata;
/* start address for the .data section. defined in linker script */
extern uint32_t _sdata;
/* end address for the .data section. defined in linker script */
extern uint32_t _edata;
/* start address for the .bss section. defined in linker script */
extern uint32_t _sbss;
/* end address for the .bss section. defined in linker script */
extern uint32_t _ebss;
/* end address for the stack. defined in linker script */
extern uint32_t _estack;


int data_test = 1;
int bss_test;
char bss_test2;
char data_test2 = 2;

static int exec_env_init(void)
{
	/* Copy the data segment initializers from flash to SRAM */
	uint32_t *idata_begin = &_sidata;
	uint32_t *data_begin = &_sdata;
	uint32_t *data_end = &_edata;

	if (idata_begin != data_begin) {
		/* init data section */
		while (data_begin < data_end) *data_begin++ = *idata_begin++;
	}

	/* Zero fill the bss segment. */
	uint32_t *bss_begin = &_sbss;
	uint32_t *bss_end = &_ebss;
	while (bss_begin < bss_end) *bss_begin++ = 0;

	if (data_test != 1 || data_test2 != 2)
		return -1;

	if (bss_test != 0 || bss_test2 != 0)
		return -2;

	return 0;
}

void usertask(void)
{
	puts("User Task #1\n");

	puts("I'm going to return to kernel mode\n");
	syscall();

	puts("Hello, I am back\n");
	puts("Let's go to return to kernel mode again\n");
	syscall();

	/* Never terminate the task */
	while (1){
		puts("Task #1 alive\n");
		timer_delay_awhile(0x100000);
		syscall();
	}
}

void usertask2(void)
{
	puts("User Task #2\n");

	/* Never terminate the task */
	while (1){
		puts("Task #2 alive\n");
		timer_delay_awhile(0x200000);

		syscall();
	}
}

struct task_cb {
	unsigned int	*stack;
};

#define STACK_DEPTH	256
#define STACK_BOUND	16
unsigned int usertask_stack[STACK_DEPTH];
struct task_cb tcb1;

unsigned int usertask_stack2[STACK_DEPTH];
struct task_cb tcb2;

void initialize_stack(struct task_cb *tcb, void (*task)(void))
{
	unsigned int *stack = tcb->stack;
	/*
	 * Initialization of process stack.
	 * r0-r12, lr, cpsr
	 */
	stack[0] = 0x0000; /* TODO: passing argument */
	stack[1] = 0x0101;
	stack[2] = 0x0202;
	stack[3] = 0x0303;
	stack[4] = 0x0404;
	stack[5] = 0x0505;
	stack[6] = 0x0606;
	stack[7] = 0x0707;
	stack[8] = 0x0808;
	stack[9] = 0x0909;
	stack[10] = 0x0a0a;
	stack[11] = 0x0b0b;
	stack[12] = 0x0c0c;
	stack[13] = (unsigned int) task;
	stack[14] = USER_MODE;
}

void main(void)
{
	if (exec_env_init()) {
		puts("env init fail\n");
		goto idle;
	}

	/* TODO: allocate stack memory */
	tcb1.stack = usertask_stack + STACK_DEPTH - STACK_BOUND;
	initialize_stack(&tcb1, usertask);

	tcb2.stack = usertask_stack2 + STACK_DEPTH - STACK_BOUND;
	initialize_stack(&tcb2, usertask2);

	puts("Hello, Jonas.\n");

	puts("Kernel: user task 1st round\n");
	tcb1.stack = activate(tcb1.stack);
	puts("Kernel: Control back\n");

	puts("Kernel: user task 2nd round\n");
	tcb1.stack = activate(tcb1.stack);
	puts("Kernel: Control back\n");

	while (1) {
		puts("Kernel: Task switch to task #1\n");
		tcb1.stack = activate(tcb1.stack);

		puts("Kernel: Task switch to task #2\n");
		tcb2.stack = activate(tcb2.stack);
	}

idle:
	puts("Kernel: Idle loop\n");
	while (1);
}
