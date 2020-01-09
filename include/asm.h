#ifndef __ASM_H__
#define __ASM_H__

#include <stdint.h>

/*
 * activate an user task
 * return value: current stack pointer
 */
uint32_t *activate(uint32_t *stack);

#endif
