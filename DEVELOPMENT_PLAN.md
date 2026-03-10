# DEVELOPMENT_PLAN.md

## Overview

This plan extends the project in two complementary directions:

1. **AArch64 platform support** — port the kernel to run on QEMU `virt` (Cortex-A53/A57) under AArch64, demonstrating how the same OS concepts map to the 64-bit ARM architecture
2. **Explicit bootloader stage** — separate the system into a distinct BL (bootloader) binary and a kernel binary, with a well-defined handoff protocol, making the boot sequence a first-class teachable artifact

All work follows **Test-Driven Development**: write a failing test (or a failing QEMU integration assertion), then implement just enough to make it pass.

---

## Roadmap

### Milestones

| Milestone | Phases | Deliverable |
|---|---|---|
| **M0** Foundation       | Phase 0       | Multi-arch build system; all existing arm32 tests green |
| **M1** AArch64 Baseline | Phases 1 – 3  | CPU boots to C, UART output, MMU on, physical pages allocatable |
| **M2** AArch64 Kernel   | Phases 4 – 6  | GICv2 live, timer preempts tasks, EL0/EL1 context switch |
| **M3** AArch64 User Space | Phases 7 – 8 | `svc` system calls work, user ELF loads and runs in EL0 |
| **M4** Bootloader       | Phase 9       | BL0 binary hands off to kernel; full two-stage boot sequence |
| **M5** Complete         | Phase 10      | Both platforms pass `scripts/run_tests.sh` unattended |

---

### Phase Dependency Graph

Each phase depends on the one(s) directly above it. Phases 7 and 8 are
independent of each other and can proceed in parallel once Phase 6 is done.

```
Phase 0  ──  Restructure (foundation for everything)
   │
Phase 1  ──  AArch64 Bootstrap (EL drop, stack, UART)
   │
Phase 2  ──  MMU (identity map, TTBR0/TTBR1 split)
   │
Phase 3  ──  Page Allocator (bitmap, 64-bit PFNs)
   │
Phase 4  ──  GICv2 + Exception Vectors
   │
Phase 5  ──  ARM Generic Timer (preemption tick)
   │
Phase 6  ──  Context Switch + Task Management (EL0/EL1)
   │         │
Phase 7      Phase 8  ──  (parallel)
SVC calls    ELF Loader
   │         │
   └────┬────┘
        │
    Phase 9  ──  Bootloader (BL0 → kernel handoff)
        │
    Phase 10 ──  CI / End-to-End Polish
```

---

### Timeline

Phases are ordered strictly by dependency. The approximate relative weight
of each phase is shown by the bar length.

```
M0               M1                        M2                  M3        M4   M5
│                │                          │                   │          │    │
▼                ▼                          ▼                   ▼          ▼    ▼
┌──────┬─────────┬─────────┬───────────────┬──────────┬────────┬───┬──────┬────┐
│  P0  │   P1    │   P2    │      P3       │    P4    │   P5   │P6 │ P7+8 │P9  │P10
│Restr.│ Boot/EL │   MMU   │  Page Alloc   │  GICv2   │ Timer  │CTX│ User │ BL │CI │
└──────┴─────────┴─────────┴───────────────┴──────────┴────────┴───┴──────┴────┘
  ^^^    ^^^^^^^^   ^^^^^^    ^^^^^^^^^^^^^    ^^^^^^^^   ^^^^^   ^^^  ^^^^  ^^^  ^^
  easy   moderate   hard      moderate         moderate   easy    hard hard  hard med
```

**Critical path:** P0 → P1 → P2 → P3 → P4 → P5 → P6 → P9 → P10

P7 (SVC) and P8 (ELF Loader) are both unblocked by P6 and feed into P9.
Neither blocks the other, so they can be developed concurrently.

---

### TDD Gate Summary (quick reference)

| Phase | Write test first | Pass condition |
|---|---|---|
| P0 | — | `make test` no regressions |
| P1 | `test_el_drop.c`, `test_pl011.c` | `[BL] UART OK` on serial |
| P2 | `test_mmu.c` | `[KERN] MMU ON` on serial |
| P3 | `test_mm64.c` | host unit tests pass |
| P4 | `test_gicv2.c` | `[IRQ] GIC OK` on serial |
| P5 | `test_arm_timer.c` | `[TICK]` ×3 on serial |
| P6 | `test_task.c` | two tasks interleave on serial |
| P7 | `test_syscall.c` | `hello from EL0` on serial |
| P8 | `test_loader64.c` | entry point + section data correct |
| P9 | `test_handoff.c` | full boot sequence on serial |
| P10 | — | `scripts/run_tests.sh` exits 0 |

---

## Target Platform: QEMU `virt` (AArch64)

| Resource | Value |
|---|---|
| QEMU machine | `-M virt -cpu cortex-a53` |
| RAM base | `0x40000000` |
| PL011 UART | `0x09000000` |
| GICv2 Distributor (GICD) | `0x08000000` |
| GICv2 CPU Interface (GICC) | `0x08010000` |
| EL1 Physical Timer IRQ | `30` (PPI, GIC ID 30) |
| Reset Exception Level | EL2 (QEMU `virt` default) |
| Toolchain | `aarch64-none-elf-` |

### AArch64 vs ARMv7 Key Differences

| Concern | ARMv7 (arm32) | AArch64 (arm64) |
|---|---|---|
| Privilege levels | PL0/PL1/PL2 (User/SVC/Hyp) | EL0/EL1/EL2/EL3 |
| Exception entry | Mode-banked registers, LR_irq | Single `ELR_ELn`, `SPSR_ELn` |
| MMU descriptors | Short-descriptor (L1=4096 entries) | VMSAv8-64 (L0-L3, 512 entries each) |
| Translation base | TTBR0/TTBR1 (32-bit) | TTBR0_EL1/TTBR1_EL1 (64-bit) |
| VA split config | TTBCR.N | TCR_EL1.T0SZ / T1SZ |
| System registers | MCR/MRC | MRS/MSR |
| GP registers | r0–r15 | x0–x30 + SP_EL0 |
| Context switch | CPSR + banked SP/LR | SPSR_EL1 + SP_EL0 + ELR_EL1 |
| Interrupt return | `subs pc, lr, #4` | `eret` |

---

## Directory Structure (Target)

```
arch/
  arm32/          # Existing ARMv7 code (start.S, context_switch.S, syscall.S)
  arm64/          # New AArch64 platform code
    start.S       # Reset handler, EL2→EL1 drop, exception vectors
    context_switch.S
    syscall.S
    mmu.c / mmu.h # AArch64 page table management
    mm.c / mm.h   # Page allocator (ported, parameterized)
bootloader/
  arm64/
    bl_start.S    # Bootloader entry, stack, UART, jumps to bl_main()
    bl_main.c     # Hardware init, copy kernel, verify, handoff
    bl.ld         # Bootloader linker script
common/           # Architecture-independent kernel logic
  irq.c / irq.h
  loader.c / loader.h
  task.h
drivers/
  pl011_uart.c / .h   # PL011 driver (shared between arm32 and arm64 configs)
  gicv2.c / .h        # GICv2 driver (arm64 uses this; arm32 keeps existing gic.c)
  arm_timer.c / .h    # ARM generic timer (arm64)
  sp804_timer.c / .h  # SP804 timer (arm32, existing)
tests/
  arm64/          # New host-side unit tests for arm64 modules
  (existing arm32 tests stay in tests/)
userspace/
  arm64/          # AArch64 user-space ELF binary
```

---

## Phase 0 — Project Restructure

**Goal:** Move existing arm32 code under `arch/arm32/` and `drivers/` without changing any behavior. All existing tests must continue to pass.

### TDD Gate
Run `make test` before and after. Zero regressions.

### Tasks

1. Create `arch/arm32/` and move `start.S`, `context_switch.S`, `syscall.S`, `mm.c`, `mm.h`, `loader.c`
2. Create `drivers/` and move `driver/uart.c`, `driver/gic.c`, `driver/sp804_timer.c` and their headers
3. Create `common/` and move `irq.c`, `irq.h`
4. Update `Makefile` and all `#include` paths accordingly
5. Add a top-level `ARCH` variable to the Makefile (`ARCH ?= arm32`) — all subsequent phases gate on this
6. Verify `make ARCH=arm32 all`, `make ARCH=arm32 qemu`, `make test` all pass unchanged

---

## Phase 1 — AArch64 Bootstrap: Reset to `init()`

**Concept demonstrated:** How the CPU starts, how exception levels work, and how to reach C from assembly on AArch64.

### TDD Gate
**Integration test:** `make ARCH=arm64 qemu` produces the string `[BL] UART OK` on serial output before halting. Automated with:
```bash
timeout 5 qemu-system-aarch64 ... | grep -q "\[BL\] UART OK" && echo PASS
```

### Tasks

1. Write `tests/arm64/test_el_drop.c` (host-side): verifies that the SPSR_EL2 value constructed by `start.S` encodes EL1h mode and masks all interrupts — test the bitmask logic as a pure C function
2. Write `arch/arm64/start.S`:
   - Install exception vector table (VBAR_EL1) with stubs that loop forever (to be replaced later)
   - If entered at EL2: configure HCR_EL2 (set RW=1 for AArch64 EL1), prepare ELR_EL2/SPSR_EL2, `eret` to EL1
   - Set up stack pointer (SP_EL1) to a symbol defined in the linker script
   - Call `init()` in C
3. Write `arch/arm64/arch64.ld` linker script: kernel loads at `0x40000000`, stack at top of a 4KB `.stack` section
4. Write `drivers/pl011_uart.c` (base address from a `#define` so it works for both arm32 and arm64)
5. Write `tests/arm64/test_pl011.c` (host-side): test `uart_putchar` logic with a mock register write function
6. Write a minimal `init()` that calls `uart_init()` then `puts("[BL] UART OK\n")` then loops — integration test passes

---

## Phase 2 — AArch64 MMU: Identity Map + Kernel High Map

**Concept demonstrated:** VMSAv8-64 page table format, TTBR0/TTBR1 split, TCR_EL1 configuration.

AArch64 uses up to 4 levels (L0–L3) of 512-entry tables. For this kernel:
- TTBR1_EL1 covers `0xFFFF_0000_0000_0000+` (kernel)
- TTBR0_EL1 covers `0x0000_0000_0000_0000+` (user, per-task)
- Use **2MB block entries** (L2) for the initial identity map and kernel map — simpler, and sufficient for this scale

### TDD Gate
**Host-side test:** `tests/arm64/test_mmu.c` — test the functions that construct L0/L1/L2 descriptors:
- Verify a 2MB block descriptor for address X has the correct output address and attribute bits
- Verify `mmu_map_block()` writes the correct value to the correct table index

**Integration test:** QEMU serial shows `[KERN] MMU ON` — kernel continues to run with translation enabled.

### Tasks

1. Write `tests/arm64/test_mmu.c` — pure C tests for descriptor-building functions (no hardware needed)
2. Write `arch/arm64/mmu.c` / `mmu.h`:
   - `mmu_init()`: zero translation tables, build L1 (1GB) and L2 (2MB) block entries for the kernel image physical region and peripherals, set TTBR0_EL1 and TTBR1_EL1, configure TCR_EL1 (T0SZ=25, T1SZ=25, 4KB granule), enable MMU via SCTLR_EL1
   - `mmu_map_block(uint64_t *table, uint64_t vaddr, uint64_t paddr, uint64_t attrs)`: write one L2 block entry
   - `mmu_set_user_table(uint64_t *pgd)`: write TTBR0_EL1 + `isb`
3. Allocate translation table memory in the linker script (aligned 4KB section `.ttb`)
4. Call `mmu_init()` from `init()` after UART; print `[KERN] MMU ON` to verify

---

## Phase 3 — AArch64 Page Allocator

**Concept demonstrated:** Physical memory management; the same bitmap allocator logic is architecture-independent.

### TDD Gate
`tests/arm64/test_mm64.c` (host-side): same test coverage as `tests/test_mm.c` but parameterized for AArch64 physical memory layout (`0x40000000` base, 256MB pool).

### Tasks

1. Write `tests/arm64/test_mm64.c` (copy and adapt `tests/test_mm.c` for 64-bit PFN arithmetic and new base address)
2. Write `arch/arm64/mm.c` / `mm.h`:
   - Same bitmap allocator design as arm32 `mm.c`
   - `uint64_t allocate_page(int count)` — returns PFN as 64-bit
   - `void free_page(uint64_t pfn, int count)`
   - `void mm_init(void)`
   - `void *map_pages(void *vaddr, uint64_t start_pfn, uint32_t count)` — creates L3 PTE entries (4KB) in the current task's TTBR0 table
3. Physical memory layout for arm64:

   | Region | Physical |
   |---|---|
   | Kernel image | `0x40000000` |
   | Translation tables | `0x4F000000` |
   | Page pool | `0x50000000` (256MB) |
   | Peripherals | `0x09000000` (UART), `0x08000000` (GIC) |

---

## Phase 4 — AArch64 Interrupt Handling: GICv2 + Exception Vectors

**Concept demonstrated:** AArch64 exception vector table layout (4 classes × 4 vectors = 16 entries, each 0x80 bytes apart), GICv2 programming model.

### TDD Gate
**Host-side test:** `tests/arm64/test_gicv2.c` — test GIC register offset calculations and IRQ enable/disable bitmask logic as pure C.

**Integration test:** QEMU serial shows `[IRQ] GIC OK` after enabling a software-generated interrupt (SGI 0) and catching it.

### Tasks

1. Write `tests/arm64/test_gicv2.c` — verify GICD_ISENABLER register index and bit position calculations for given IRQ numbers
2. Replace stub exception vectors in `arch/arm64/start.S` with real handlers:
   - `el1_sync_handler`: reads ESR_EL1 to distinguish SVC, data abort, instruction abort
   - `el1_irq_handler`: reads GIC IAR, calls `irq_top_handler()`, writes EOIR
   - Others: print error and halt
3. Write `drivers/gicv2.c` / `gicv2.h`:
   - `gicv2_init()`: enable GICD, set GICC priority mask to `0xFF`, enable GICC
   - `gicv2_enable_irq(uint32_t irq)`: set bit in GICD_ISENABLER
   - `gicv2_clear_pending(uint32_t irq)`: write GICD_ICPENDR
   - `gicv2_ack()` / `gicv2_eoi(uint32_t irq)`: IAR read / EOIR write
4. Wire `irq_top_handler()` from `common/irq.c` into the EL1 IRQ vector
5. Integration test: trigger SGI 0, verify handler is called, print `[IRQ] GIC OK`

---

## Phase 5 — AArch64 Timer: Preemption Tick

**Concept demonstrated:** ARM generic timer (a core CPU timer, different from the SP804 board timer), using system registers instead of MMIO.

The ARM generic timer is accessed via `CNTV_CTL_EL0`, `CNTV_CVAL_EL0`, `CNTVCT_EL0` (virtual timer) or the EL1 physical timer via `CNTP_*_EL0`. IRQ line 30 (PPI) on QEMU virt.

### TDD Gate
**Host-side test:** `tests/arm64/test_arm_timer.c` — test the `tval` calculation: given a frequency and desired period in ms, verify the computed countdown value.

**Integration test:** QEMU serial prints `[TICK]` at least 3 times before the test harness times out.

### Tasks

1. Write `tests/arm64/test_arm_timer.c`
2. Write `drivers/arm_timer.c` / `arm_timer.h`:
   - `arm_timer_init(uint32_t period_ms)`: read `CNTFRQ_EL0`, compute TVAL, set `CNTP_TVAL_EL0`, enable via `CNTP_CTL_EL0`
   - `arm_timer_handler()`: reloads TVAL, sets `do_context_switch = 1`, clears interrupt
   - `arm_timer_setup_irq()`: call `gicv2_enable_irq(30)` + register handler
3. Enable timer PPI in GICv2 (PPIs use GICD_ISENABLER[0], bits 16–31)
4. Call `arm_timer_init()` from `init()`, verify `[TICK]` output

---

## Phase 6 — AArch64 Task Management & Context Switch

**Concept demonstrated:** AArch64 exception return to EL0 (User), `SP_EL0` vs `SP_EL1`, saving/restoring the full AArch64 register file.

AArch64 context on the stack (pushed by the EL1 IRQ entry / `activate`):
```
[x0–x30]   (31 × 8 bytes)
[SP_EL0]   (8 bytes)
[ELR_EL1]  (8 bytes) ← return PC
[SPSR_EL1] (8 bytes) ← return CPSR, must encode EL0t (0x0)
```

### TDD Gate
**Host-side test:** `tests/arm64/test_task.c` — given a task entry address and a stack buffer, call `initialize_stack64()` and verify that the resulting stack pointer points to a frame with the correct `ELR_EL1` and `SPSR_EL1` values (EL0 mode bits).

### Tasks

1. Write `tests/arm64/test_task.c`
2. Update `include/task.h` to be pointer-width agnostic (use `uintptr_t` instead of `uint32_t` for `stack`, `pte`, `vaddr`)
3. Write `arch/arm64/context_switch.S`:
   - `activate(uintptr_t *stack)`: restores x0–x30, SP_EL0, ELR_EL1, SPSR_EL1 from the stack frame, calls `eret` to EL0
   - On IRQ entry (inside `el1_irq_handler`): save x0–x30, SP_EL0, ELR_EL1, SPSR_EL1; update `current->stack`; after `irq_top_handler()` returns, call `activate(current->stack)` for the next task
4. Write `arm64/main.c`:
   - `initialize_stack64(uint64_t *stack_top, uint64_t entry)`: fills the frame layout above with EL0t SPSR
   - `main()`: initializes two task TCBs, calls `activate()` for the first task, re-enters scheduler on each tick
5. Unit test `initialize_stack64()` with `tests/arm64/test_task.c` before writing `main.c`

---

## Phase 7 — AArch64 System Calls (EL0 → EL1 via `svc`)

**Concept demonstrated:** On AArch64, `svc #0` from EL0 causes a synchronous exception to EL1. `ESR_EL1.EC = 0x15` identifies it as an SVC. The system call number is in ESR_EL1.ISS (the immediate operand of `svc`), and arguments arrive in x0–x5.

### TDD Gate
**Host-side test:** `tests/arm64/test_syscall.c` — test the ESR_EL1 parsing logic: given a raw ESR value, verify that EC extraction and ISS extraction return expected values.

**Integration test:** User task calls `syscall(0, "hello from EL0\n", buf)`, kernel SVC handler copies the string and prints it — QEMU serial shows `hello from EL0`.

### Tasks

1. Write `tests/arm64/test_syscall.c`
2. Write `arch/arm64/syscall.S`: userspace-side `syscall(service, arg1, arg2)` — places args in x0/x1/x2 and executes `svc #0`
3. In `el1_sync_handler` (start.S): check `ESR_EL1.EC == 0x15`, call `do_system_service(x0, x1, x2)`
4. `is_user_ptr()`: validate pointer is below `0x0001_0000_0000` (user VA range)
5. Port user-space hello program to AArch64 (`userspace/arm64/`)
6. Integration test

---

## Phase 8 — ELF Loader Port to AArch64

**Concept demonstrated:** ELF loader is largely architecture-independent; the difference is mapping to a 64-bit virtual address space.

### TDD Gate
Existing `tests/test_loader.c` (memset/memcpy/memcmp tests) must pass unchanged. Add `tests/arm64/test_loader64.c` to test `load_user_task64()` with a synthetic ELF buffer.

### Tasks

1. Write `tests/arm64/test_loader64.c`: construct a minimal ELF64 binary in a byte array, call `load_user_task64()`, verify entry point and that allocated pages contain the section data
2. Move shared memory utility functions (`memcpy`, `memset`, `memcmp`) into `common/string.c`
3. Write `common/loader.c` / `loader.h` parameterized for address size (use `uintptr_t`), replacing the arm32-specific `loader.c`
4. Wire into `arm64/main.c` `init()` — load the arm64 user ELF, set up its TTBR0, launch it

---

## Phase 9 — Bootloader Stage

**Concept demonstrated:** Real embedded systems have a multi-stage boot: a ROM bootloader (BL1) hands off to a larger bootloader (BL2/U-Boot), which loads and verifies the OS. Here we build a minimal two-stage boot:

- **Stage 0 (BL0)** — `bootloader/arm64/bl_start.S`: minimal entry, stack, basic UART, loads Stage 1
- **Stage 1 (kernel)** — the kernel binary built by the existing `ARCH=arm64` target

The BL is a separate ELF, linked to run at `0x40000000`. The kernel is linked to run at `0x40100000`. The QEMU `-bios` flag (or a combined image) is used to load BL first.

### Handoff Protocol

BL passes a `struct handoff_info` pointer in `x0` to the kernel entry point:
```c
struct handoff_info {
    uint64_t kernel_entry;       // resolved entry point
    uint64_t ram_base;
    uint64_t ram_size;
    uint64_t uart_base;          // UART physical base confirmed by BL
    uint32_t magic;              // HANDOFF_MAGIC = 0xB007B007
};
```

### TDD Gate
**Host-side test:** `tests/arm64/test_handoff.c` — verify that `bl_prepare_handoff()` populates the struct correctly for given inputs and that the kernel's `check_handoff()` validates the magic number and rejects bad structs.

**Integration test:** QEMU serial shows:
```
[BL0] bootloader start
[BL0] UART OK
[BL0] jumping to kernel at 0x40100000
[KERN] handoff OK
[KERN] MMU ON
[TICK][TICK][TICK]
hello from EL0
```

### Tasks

1. Write `tests/arm64/test_handoff.c`
2. Write `bootloader/arm64/bl_main.c`:
   - `bl_prepare_handoff()`: fill in `struct handoff_info`
   - Verify kernel magic word at expected location
   - Print `[BL0] jumping to kernel at 0x...`
   - Jump to kernel entry via function pointer (passes `handoff_info *` in x0)
3. Write `bootloader/arm64/bl_start.S`: entry at `0x40000000`, set up SP, call `bl_main()`
4. Write `bootloader/arm64/bl.ld`: place `.text` at `0x40000000`
5. Write `arch/arm64/check_handoff.c`: validates `handoff_info`, halts on bad magic
6. Update `Makefile` with `make ARCH=arm64 bl` target and `make ARCH=arm64 qemu-boot` (loads BL as `-bios` or prepends it to the combined image)

---

## Phase 10 — Integration Polish & End-to-End Demo

**Goal:** Both platforms boot to a running user task in a single `make` invocation. Automated test script passes on CI.

### Tasks

1. Write `scripts/run_tests.sh`: runs host unit tests + QEMU integration tests for both arm32 and arm64 with `timeout`, checks expected serial output with `grep`, exits non-zero on failure
2. Add `make test-all` target: `make test` (arm32) + `make ARCH=arm64 test`
3. Update `CLAUDE.md` with arm64 build commands and test script usage
4. Fix any observed regressions from the integration run

---

## TDD Conventions

### Host-Side Unit Tests (fast, no QEMU)
- Located in `tests/` (arm32) or `tests/arm64/` (arm64)
- Compiled with host `gcc`, include the `.c` file under test directly
- Mock hardware: provide mock global variables for register addresses and hardware state
- Pattern: `if (result != expected) { printf("FAIL: ..."); return 1; }` → `return fails > 0 ? 1 : 0`
- Run with `make test` / `make ARCH=arm64 test`

### QEMU Integration Tests (slower, require QEMU)
- Run with `make ARCH=arm64 qemu-test` (non-interactive, `-nographic`, piped to `grep`)
- Each phase has a single clear string to `grep` for as the pass condition
- Kernel must call `qemu_exit(0)` (write `0x20026` to `0x09000000` sysbus exit device on virt machine) after printing the test string, so the integration test terminates cleanly

### Phase Gate
No phase begins implementation until the tests for that phase are written and confirmed failing. No phase is "done" until all its tests pass and no previously passing test regresses.

---

## Summary Table

| Phase | Concept | TDD Gate |
|---|---|---|
| 0 | Refactor for multi-arch | Existing tests pass |
| 1 | AArch64 boot, EL drop, UART | `[BL] UART OK` on serial |
| 2 | VMSAv8-64 MMU, 2MB blocks | `[KERN] MMU ON` on serial |
| 3 | 64-bit page allocator | `test_mm64` host tests |
| 4 | GICv2 + exception vectors | SGI handler fires, `[IRQ] GIC OK` |
| 5 | ARM generic timer + preemption | `[TICK]` appears repeatedly |
| 6 | EL0/EL1 context switch | Two tasks interleave on serial |
| 7 | AArch64 SVC system call | `hello from EL0` on serial |
| 8 | ELF loader (64-bit) | User ELF loaded, entry correct |
| 9 | Bootloader + handoff protocol | Full boot sequence on serial |
| 10 | End-to-end, CI script | `scripts/run_tests.sh` exits 0 |
