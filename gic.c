#include "gic.h"



#define GICC_BASE	(GIC_PERIPHBASE + 0x100)
#define GICC_CTRL	(0x00)
#define GICC_PMR	(0x04)

#define GICD_BASE	(GIC_PERIPHBASE + 0x1000)
#define GICD_CTRL	(0x00)

#define GICD_ISENABLER	(0x100)
#define GICD_ICPENDR 	(0x280)
#define GICD_IPRIORITYR	(0x400)
#define GICD_ITARGETSR	(0x800)
#define GICD_ICFGR	(0xC00)

static void gic_write_cpu_reg(uint32_t val, uint32_t addr)
{
	write_reg(val, GICC_BASE + addr);
}

static void gic_write_dist_reg(uint32_t val, uint32_t addr)
{
	write_reg(val, GICD_BASE + addr);
}

static uint32_t gic_read_cpu_reg(uint32_t addr)
{
	return read_reg(GICC_BASE + addr);
}

static uint32_t gic_read_dist_reg(uint32_t addr)
{
	return read_reg(GICD_BASE + addr);
}

static void gic_enable_gicd(void)
{
	gic_write_dist_reg(0x1, GICD_CTRL);
}

static void gic_enable_gicc(void)
{
	gic_write_cpu_reg(0x1, GICC_CTRL);
}

static void gic_set_sens_type(uint32_t vector, enum gic_sensitive_type type)
{
	uint32_t offset, mask;
	uint32_t addr, value;

	offset = (vector / 32) * 4;
	addr = GICD_ICFGR + offset;

	value = gic_read_dist_reg(addr);

	mask = 0x1 << (vector % 32);

	if (type == GIC_SENS_EDGE)
		value |= mask;
	else
		value &= ~mask;

	gic_write_dist_reg(value, addr); 
}

static void gic_set_priority(uint32_t vector, uint8_t priority)
{
	uint32_t offset, mask, shift;
	uint32_t addr, value;

	offset = (vector / 4) * 4;
	addr = GICD_IPRIORITYR + offset;

	value = gic_read_dist_reg(addr);

	shift = (vector % 4) * 8;
	mask = 0xFF << shift;
	value &= ~mask;
	value |= (priority << shift);

	gic_write_dist_reg(value, addr); 
}

static void gic_set_target_cpu(uint32_t vector)
{
	uint32_t offset, mask, shift;
	uint32_t addr, value;

	offset = (vector / 4) * 4;
	addr = GICD_ITARGETSR + offset;
	
	value = gic_read_dist_reg(addr);

	shift = (vector % 4) * 8;
	mask = 0xFF << shift;
	value &= ~mask;
	value |= (0x1 << shift); /* all set to CPU-0 now */

	gic_write_dist_reg(value, addr); 
}

static void gic_clear_pending(uint32_t vector)
{
	uint32_t offset;
	uint32_t addr, value;

	offset = (vector / 32) * 4;
	addr = GICD_ICPENDR + offset;

	value = 0x1 << (vector % 32);

	gic_write_dist_reg(value, addr); 
}

static void gic_enable_irq(uint32_t vector)
{
	uint32_t offset;
	uint32_t addr, value;

	offset = (vector / 32) * 4;
	addr = GICD_ISENABLER + offset;

	value = 0x1 << (vector % 32);

	gic_write_dist_reg(value, addr); 
}

void gic_setup_irq(struct gic_irq_desc *desc)
{
	uint32_t vector;

	vector = desc->vector;

	gic_set_sens_type(vector, desc->sensitive);

	gic_set_priority(vector, desc->priority);

	gic_set_target_cpu(vector);

	gic_clear_pending(vector);

	gic_enable_irq(vector);
}

void gic_init(void)
{
	gic_enable_gicd();

	/* Enable forwording interrupts of all priority levels */
	gic_write_cpu_reg(0xFF, GICC_PMR);

	gic_enable_gicc();
}
