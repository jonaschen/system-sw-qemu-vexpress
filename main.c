#include <stdint.h>

#include "reg.h"


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
	while (1); /* Never terminate the task */
}

void initialize_stack(unsigned int *stack, void (*task)(void))
{
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
	stack[13] = (unsigned int) &usertask;
	stack[14] = 0x0000001F;
}

void activate(unsigned int *stack);

void main(void)
{
	/* Initialization of process stack.
	 * r0-r12, cpsr, lr
	 * TODO: task control block
	 */
	unsigned int usertask_stack[256];
	unsigned int *usertask_stack_start = usertask_stack + 256 - 16;

	initialize_stack(usertask_stack_start, usertask);

	if (exec_env_init()) {
		puts("env init fail\n");
		goto idle;
	}

	puts("Hello, Jonas.\n");
	activate(usertask_stack_start);

idle:
	while (1);
}
