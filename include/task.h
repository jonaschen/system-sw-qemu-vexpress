#ifndef __TAKS_H__
#define __TAKS_H__

struct task_cb {
	uint32_t	*stack;
	uint32_t	*pte;
	uint32_t	vaddr;
};

extern struct task_cb *current;

#endif
