# system-sw-qemu-vexpress
---------
This is a study project, just for fun.
All the source code is based on jserv's examples.
I have been studing jserv's tutorial materials and code samples for a while.
To express my thankfulness, I am trying to give some feedback 
and sharing of my experience.

# Project Status
The project currently implements a basic ARMv7 kernel for QEMU VExpress with the following features:

### Kernel Core
-   **Multitasking**: Preemptive round-robin scheduler.
-   **Interrupts**: ARM Generic Interrupt Controller (GIC) support.
-   **Timer**: ARM SP804 Dual Timer driver for system ticks.
-   **System Calls**: Basic `SVC` mechanism for user-kernel transition.

### Memory Management
-   **MMU**: Enabled with paging.
-   **Virtual Memory**:
    -   Split kernel (TTBR1) and user (TTBR0) address spaces.
    -   Kernel runs at `0x80000000` (virtual).
-   **Allocator**: Basic physical page allocator and `vmalloc`.

### User Space
-   **ELF Loader**: Basic support for loading statically linked ELF binaries from memory.
-   **User Mode**: Tasks run in ARM User Mode.
-   **Sample Tasks**: Demonstration of context switching and syscalls.

### Drivers
-   **UART**: PL011 UART driver for console output.

# Roadmap
Future development plans include:

1.  **Memory Management**
    -   Implement page reclamation (`free_page`).
    -   Improve ELF loader to support multi-page sections and BSS.
    -   Implement user-space heap (`malloc`/`free`).

2.  **File System**
    -   Implement a basic file system (e.g., RAMFS).
    -   Load user programs from the file system instead of raw memory.

3.  **Process Management**
    -   Implement `fork`, `exec`, `exit`, and `wait` system calls.
    -   Add support for task priorities.
    -   Implement Inter-Process Communication (IPC).

4.  **I/O & Drivers**
    -   Interrupt-driven UART driver (Input/Output).
    -   Implement a basic interactive Shell.

5.  **User Space**
    -   Develop a minimal C library (libc).
    -   Port more complex applications.

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
