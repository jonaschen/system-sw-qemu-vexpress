#include "uart.h"
#include "reg.h"
#include "gic.h"
#include "irq.h"

#define IN_BUF_SIZE	(64)
struct uart_input_buf {
	char	*head;
	char	*tail;
	int	count;
	char	buf[IN_BUF_SIZE];
};

static struct uart_input_buf input_buf;

static void uart_buf_init(void)
{
	input_buf.head = &input_buf.buf[0];
	input_buf.tail = &input_buf.buf[0];
	input_buf.count = 0;
}

char uart_buf_getchar(void)
{
	char c;

	if (input_buf.count == 0)
		return 0;

	c = *(input_buf.tail);
	input_buf.tail++;
	if (input_buf.tail == &input_buf.buf[IN_BUF_SIZE])
		input_buf.tail = &input_buf.buf[0];

	input_buf.count--;

	return c;
}

void uart_buf_putchar(char c)
{
	if (input_buf.count >= IN_BUF_SIZE)
		return;

	*(input_buf.head) = c;
	input_buf.head++;
	if (input_buf.head == &input_buf.buf[IN_BUF_SIZE])
		input_buf.head = &input_buf.buf[0];

	input_buf.count++;

	return;
}

int putchar(int c)
{
	*((unsigned int *) UART_BASE) = c;
	return 0;
}

int puts(const char *str)
{
	while (*str)
		*((unsigned int *) UART_BASE) = *str++;
	return 0;
}

void uart_int(void *arg)
{
	int c;
	uint32_t value;

//	puts("uart irq.\n");
	c = *((unsigned int *) UART_BASE);
//	*((unsigned int *) UART_BASE) = c;
	uart_buf_putchar(c);

	value = read_reg(UART_MIS);
	write_reg(value, UART_ICR);
}

#define UART_CLK_FREQ	(24000000)	/* from versatile-pb.dts */
void uart_init(void)
{
	struct gic_irq_desc desc;
	uint32_t vector = IRQ0_UART;
	uint32_t value;

	desc.vector = vector;
	desc.sensitive = GIC_SENS_LEVEL;
	desc.priority = 0x80;

	irq_register_handler(vector, uart_int, 0);
	gic_setup_irq(&desc);

	value = read_reg(UART_IMSC);
	value |= (UART_IMSC_RXIM);
	write_reg(value, UART_IMSC);

	uart_buf_init();
}
