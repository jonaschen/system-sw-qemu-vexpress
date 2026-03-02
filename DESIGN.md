# System Design Overview

## Architecture
This project implements a basic ARMv7 operating system kernel targeted for the QEMU VExpress (vexpress-a9) board. The kernel is designed around a preemptive, round-robin multitasking scheduler running in privileged modes, while user tasks execute in unprivileged ARM User Mode. It demonstrates fundamental OS concepts, including memory management with virtual memory (MMU), hardware interrupts, system calls, and basic driver support.

The system is compiled using the `arm-none-eabi-` bare-metal GCC toolchain and heavily utilizes a freestanding C environment (`-ffreestanding`, `-nostdlib`).

## Memory Management (MMU)
The memory management subsystem leverages the ARMv7 short-descriptor translation table format.
Virtual memory is split between user space and kernel space at `0x80000000` (`KERNEL_VIRT_BASE`).
- **User Space (TTBR0)**: User processes are dynamically mapped and loaded here.
- **Kernel Space (TTBR1)**: The kernel image runs at a high virtual address (`0x80000000`).

### Physical Memory Layout
According to `kernel.ld` and `mm.c`:
- **Kernel Image**: Physical starting address `0x60000000` (length `0x10000000`).
- **Translation Tables (TTBDIR)**: Physical starting address `0x6F000000`.
- **Page Allocator (RAM2)**: Physical starting address `0x70000000` (`PHYSICAL_START_FRAME`).
- **Peripherals**: Physical starting address `0x10000000` (e.g., UART base, GIC base, Timer base).

## Multitasking & Scheduler
The kernel uses a preemptive round-robin scheduler to manage tasks.
- **Task Control Block (TCB)**: Each task is represented by a `struct task_cb`, containing the stack pointer, current status, ID, and memory management properties (like `pgd_dir`).
- **Timer Interrupt**: Preemption is driven by the SP804 timer (`TICK_INTERVAL`), which raises an IRQ at regular intervals to trigger context switches.
- **Context Switch**: The system saves the CPU registers to the current task's stack and restores the registers from the next task's stack. The `current` pointer tracks the active task.

## Interrupt Handling
Interrupts are managed by the ARM Generic Interrupt Controller (GIC).
- An IRQ vector handles incoming hardware interrupts.
- Registered interrupt handlers are kept in an array `irq_handlers[MAX_IRQ_NUM]`.
- The main interrupt source currently used is the SP804 dual timer for system ticks.

## System Calls (SVC)
System calls allow user-space applications to request privileged operations.
- Executed via the `svc 0` instruction.
- The `do_system_service` handler intercepts the call, validates pointers (using `is_user_ptr`), and performs operations safely without exposing kernel memory.
- Example calls include simple serial output for user processes.

## Drivers
The kernel includes minimal driver support for essential hardware:
- **PL011 UART**: Used for basic polled console output (`printf`, `puts`).
- **SP804 Dual Timer**: Provides the periodic system tick for the scheduler.
- **ARM GIC**: Handles routing of hardware interrupts.

## User Space Execution Flow
User programs are built statically as ELF binaries.
1. The kernel's basic ELF loader reads the binary from the `_sfirmware` region in memory.
2. The loader sets up a separate page directory (PGD) for the user task and maps its text/data sections.
3. The kernel transitions to the task via `activate()`, dropping privilege to ARM User Mode.
4. The user program can invoke system calls via `svc` instructions to interact with the kernel.
