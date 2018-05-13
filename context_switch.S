#include "processor.h"

.global activate

activate:
	/* TODO: save correct kernel state - svc mode now */
	push 	{r4-r12, lr}

	/* switch to process stack */
	cps	#SYSTEM_MODE

	/* load user state */
	mov 	sp, r0
	pop 	{r0-r12}

	/* jump to user task, pop lr and cpsr from stack */
	rfeia	sp!
