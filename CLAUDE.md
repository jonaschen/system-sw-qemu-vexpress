# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
make all        # Build kernel image, user programs, and test binary
make qemu       # Build and run kernel in QEMU (Ctrl-A then X to exit)
make test       # Build and run host-side unit tests
make clean      # Clean all build artifacts
```

**Toolchain**: `arm-none-eabi-` (bare-metal GCC). Dependencies: `gcc-arm-none-eabi`, `bc`, `qemu-system-arm`.

**Debug with GDB**:
```bash
# In one terminal: add -S -s to the qemu command in Makefile
qemu-system-arm -M vexpress-a9 -m 1024M -nographic -kernel out/test.elf -S -s
# In another terminal:
arm-none-eabi-gdb out/kernel.elf
(gdb) target remote localhost:1234
```

## Tests

Tests compile for the **host system** (standard `gcc`, not bare-metal). Individual test binaries after `make test`:
```bash
./tests/test_irq
./tests/test_loader
./tests/test_mm
```

## Architecture

This is a bare-metal ARMv7 kernel for QEMU VExpress (vexpress-a9) implementing preemptive multitasking, MMU-based virtual memory, hardware interrupts, and system calls.

### Boot and Execution Flow

1. `start.S` — ARM exception vector table, boot code (stack init, vector base, MMU setup, 1MB section mappings), exception handlers (SVC, IRQ, abort, etc.)
2. `main.c` — Kernel entry: initializes TCBs and driver stack, runs the round-robin scheduler loop
3. `context_switch.S` — Saves/restores CPU registers between tasks
4. `syscall.S` — Handles `svc` instructions; validates user pointers before calling `do_system_service()`

### Memory Layout

Virtual memory split at `0x80000000` (`KERNEL_VIRT_BASE`), using ARMv7 short-descriptor translation tables (TTBR0/TTBR1 split):

| Region | Physical Address |
|--------|-----------------|
| Kernel image | `0x60000000` |
| Translation tables (TTBDIR) | `0x6F000000` |
| Page allocator pool | `0x70000000` (`PHYSICAL_START_FRAME`) |
| Peripherals | `0x10000000` |

`mm.c` provides: `mm_init()`, `allocate_page(count)`, `free_page(pfn, count)`, `map_pages()`, `switch_user_pte()`.

### User Space Loading

User programs are static ELF binaries embedded in `_sfirmware` region of `test.elf` (at offset `0x10100` past the kernel ELF). `loader.c` parses the ELF, allocates physical pages, maps them into the task's TTBR0 page directory, and sets the task entry point.

### Key Structures

- `struct task_cb` (`include/task.h`) — Task Control Block: stack pointer, status, ID, `pgd_dir` (page directory), `vaddr`
- `irq_handlers[]` (`irq.c`) — Array of handlers registered via `irq_register_handler()`
- `current` — Global pointer to the active task

### Drivers (`driver/`)

- `uart.c` — PL011 UART, polled output
- `gic.c` — ARM Generic Interrupt Controller
- `sp804_timer.c` — SP804 Dual Timer (system tick driving preemption via `TICK_INTERVAL`)
