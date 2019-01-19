# system-sw-qemu-vexpress
---------
This is a study project, just for fun.
All the source code is based on jserv's examples.
I have been studing jserv's tutorial materials and code samples for a while.
To express my thankfulness, I am trying to give some feedback 
and sharing of my experience.

# Done
a. Multitasking

b. ARM's Generic Timer (Cortex-A9 doesn't have generic timer)

	ARM Dual-Timer Module (SP804) Technical Reference Manual
	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0271d/index.html

c. ARM's Generic Interrupt Controller

   c-1. Trigger timer interrupt and enter irq mode (commit: fb0be96)

d. System tick & Preemptive

e. MMU enabled (with identity mapping)

# Next
---------
a. Add some syscall services

b. Improve task processing utilities

c. Enable cache & take use of MMU

c-1. Use Kernel Virtual Address from offset 0x80000000, so we can take use of TTBR1

c-2. Simple Page allocator

c-3. User Hello World


Licensing
---------
`system-sw-qemu-vexpress` is freely redistributable under the two-clause BSD License. Use of this source code is governed by a BSD-style license that can be found in the `LICENSE` file.

# Reference
---------
jserv's mini-arm-os - https://github.com/jserv/mini-arm-os

jserv's Hello World for bare metal ARMv8 using QEMU
 - https://github.com/jserv/armv8-hello.git


ESLab/FreeRTOS---ARM-Cortex-A9-VersatileExpress-Quad-Core-port

https://github.com/jonaschen/FreeRTOS---ARM-Cortex-A9-VersatileExpress-Quad-Core-port.git

linux source - vexpress-v2m.dtsi
http://infocenter.arm.com/help/topic/com.arm.doc.ddi0183g/DDI0183G_uart_pl011_r1p5_trm.pdf
http://infocenter.arm.com/help/topic/com.arm.doc.dui0448i/DUI0448I_v2p_ca9_trm.pdf
http://infocenter.arm.com/help/topic/com.arm.doc.dui0447j/DUI0447J_v2m_p1_trm.pdf
