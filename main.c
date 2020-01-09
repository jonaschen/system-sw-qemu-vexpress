#include <stdint.h>

#include "reg.h"
#include "asm.h"
#include "processor.h"
#include "sp804_timer.h"
#include "gic.h"
#include "uart.h"
#include "mm.h"
#include "task.h"
#include "io.h"
#include "syscall.h"

/* start address for the initialization values of the .data section.
defined in linker script */
extern uint32_t _etext;
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
	uint32_t *idata_begin = &_etext;
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
	int a = 0, b = -1, c = 3;
	int sysno;

	puts("User Task #1\n");

	/* Never terminate the task */
	while (1){
		puts("Task #1 alive\n");
		timer_delay_awhile(0x80000);

		sysno = 16;
		do_syscall0(sysno);

		sysno = 17;
		do_syscall1((void *)&c, sysno);
		if (c == 0)
			puts("syscall1 test OK\n");

		sysno = 18;
		do_syscall2((void *)&a, (void *)&c, sysno);
		if (c == a)
			puts("syscall2 test OK\n");

		sysno = 19;
		do_syscall3((void *)&a, (void *)&b, (void *)&c, sysno);
		if (a == b + c)
			puts("syscall3 test OK\n");

		a += 2;
		c = a + 1;
	}
}

void uart_task(void)
{
	char c;

	puts("UART Task\n");

	/* Never terminate the task */
	while (1){
		timer_delay_awhile(0x100);
		while ((c = uart_buf_getchar()) != 0)
			putchar((int) c);

		do_syscall0(100);
	}
}

void do_system_service(int service, void *arg1, void *arg2, void *arg3)
{

	switch (service) {
	case 17:
		*(int *) arg1 = 0;
		break;
	case 18:
		*(int *) arg2 = *(int *) arg1;
		break;
	case 19:
		*(int *) arg1 = *(int *) arg2 + *(int *) arg3;
		break;
	default:
		break;
	}

	return;
}

#define STACK_DEPTH	256
#define STACK_BOUND	16
#define KERNEL_TASK_NUM	(2)
#define USER_TASK_NUM	(2)
#define TASK_NUMBER	(KERNEL_TASK_NUM + USER_TASK_NUM)

uint32_t usertask_stacks[TASK_NUMBER][STACK_DEPTH];
struct task_cb tcbs[TASK_NUMBER];

struct task_cb *current;

void initialize_stack(struct task_cb *tcb, void (*task)(void))
{
	unsigned int *stack = (unsigned int *) tcb->stack;
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
	stack[13] = 0x0d0d;
	stack[14] = (unsigned int) task;
	stack[15] = USER_MODE;
}

int do_context_switch = 0;

extern int load_user_task(struct task_cb *tcb, uint32_t *task_entry);
void main(void)
{
	uint32_t task_entry;
	int i;

	for (i = 0; i < TASK_NUMBER; i++) {
	/* TODO: allocate stack memory */
		tcbs[i].stack = usertask_stacks[i] + STACK_DEPTH - STACK_BOUND;
	}
	initialize_stack(&tcbs[0], usertask);
	initialize_stack(&tcbs[1], uart_task);

	for (i = KERNEL_TASK_NUM; i < TASK_NUMBER; i++) {
		current = &tcbs[i];
		load_user_task(&tcbs[i], &task_entry);
		initialize_stack(&tcbs[i], (void *) task_entry);
	}

	setup_timer_irq();
	timer_periodic_setup(0x100000);

	while (1) {
		for (i = 0; i < TASK_NUMBER; i++) {
			puts("Kernel: Task switch to #");
			print_int((uint32_t) i + 1);
			puts("\n");
			tcbs[i].stack = activate(tcbs[i].stack);
		}
	}

idle:
	puts("Kernel: Idle loop\n");
	while (1);
}

static void page_allocate_test(void)
{
	int result = 0, cnt = 1, i;
	void *vaddr;

	vaddr = get_page(cnt);
	if (!vaddr) {
		result = -1;
		goto out;
	}

	for (i = 0; i < 1024; i++)
		*((uint32_t *)vaddr + i) = 0x55ff00aa;

	for (i = 0; i < 1024; i++) {
		if (*((uint32_t *)vaddr + i) != 0x55ff00aa) {
			puts("page alloc test fail\n");
			while (1);
		}
	}
out:
	if (result) {
		puts("page alloc test fail. loop forever\n");
		while (1);
	}
}

void init(void)
{

	if (exec_env_init()) {
		puts("env init fail\n");
		return;
	}

	mm_init();

	page_allocate_test();

	gic_init();
	uart_init();

	main();
}
