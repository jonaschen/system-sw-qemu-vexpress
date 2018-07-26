#ifndef	__GIC_H__
#define	__GIC_H__

#include "reg.h"

enum gic_sensitive_type {
	GIC_SENS_EDGE = 0,
	GIC_SENS_LEVEL,
};

struct gic_irq_desc {
	uint32_t	vector;
	uint8_t		priority;

	enum gic_sensitive_type		sensitive;
};

void gic_setup_irq(struct gic_irq_desc *desc);
void gic_init(void);


#endif
