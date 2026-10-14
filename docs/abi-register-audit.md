# ABI register audit

Audit of the assembly that implements `stackman_switch` and `stackman_call`.
The question is whether each routine spills every non-volatile register to the
stack before calling out, and loads that image back before returning.

`stackman_switch` must spill the whole non-volatile set. The callback may
copy or replace the stack, and the epilogue may return to a different caller.
`stackman_call` returns to the same caller, so it only has to preserve the
non-volatile registers it modifies.

Volatile (caller-saved) registers are free to change across either call.
A test that expects them to keep their values is checking the wrong contract.

Sources: AAPCS64 (2025Q4 / 2026Q3), Microsoft ARM64 and ARM32 ABI notes,
Microsoft x64 calling convention, System V i386 and AMD64 ABIs, RISC-V psABI
LP64D.

## Existing documentation

There is no file that lists, for each ABI, which registers `stackman_switch`
spills. Those lists live in the assembly and in the clobber comments of the
inline headers. This note is the cross-check.

The API is described in two places:

- `README.md`, section "API", describes both functions. The `stackman_call`
  steps now say that the callback receives the previous stack pointer. The
  "Features" list now tells a new port to save callee-saved registers. It
  previously said "volatile", which is the wrong set.
- `stackman/stackman_switch.h` is the implementation contract. Steps 1–6 for
  `stackman_switch` match the assembly: spill callee-saved state, call with
  `STACKMAN_OP_SAVE`, install the returned stack pointer, call with
  `STACKMAN_OP_RESTORE`, pop the state, return the second result. The
  `stackman_call` comment says the third argument is the previous stack
  pointer. The `STACKMAN_OP_CALL` enumerator comment said the opposite (the
  pointer passed into `stackman_call`). It now matches the function comment.
  `switch_riscv64_gcc.S` now follows it: a null stack is left unchanged,
  and the callback receives the previous stack pointer.

Per-platform comments, checked against the code:

| Place | Claim | Matches the code |
| --- | --- | --- |
| `switch_x86_64_gcc.h` | `rbx`, `rbp`, `r12`–`r15`, x87 control word, MXCSR | Yes. The comment used to say "mmx"; the instruction is `stmxcsr`. |
| `switch_x86_gcc.h` | `ebx`, `esi`, `edi`, `ebp` via the frame, x87 control word | Yes. The control word is stored with `fstcw` at the pointer passed to the callback. |
| `switch_aarch64_gcc.h` | `x19`–`x28`, `x29` via the frame, `d8`–`d15` | Yes for the base AAPCS64 set. Silent on `x18` and SME. |
| `switch_arm_gcc.h` | `r4`–`r11`, and `d8`–`d15` unless Thumb. `FPSCR` is not saved. | The integer list matches `switch_arm_gcc.S`. `CP_REGISTERS` is the inline clobber only; that `.S` is soft-float and does not save `d8`–`d15`. `__ARM_PCS_VFP` is a compile error. |
| `switch_riscv64_gcc.S` | `s0`–`s11`, `fs0`–`fs11`, `ra` for `stackman_switch` | Yes for that function. `stackman_call` skips a null stack and passes the previous stack pointer. |
| `switch_x64_msvc.asm` | non-volatile GPRs, `xmm6`–`xmm15`, and MXCSR | Yes. The control bits of MXCSR are saved with the xmm frame. Upper ymm/zmm bits stay volatile. |
| `switch_arm64_msvc.asm` | `x18`–`x28`, `d8`–`d15`, `FPCR` are pushed | The spill matches the comment. `x18` is spilled and restored, and neither routine writes the live register. The callback is held in `x20`. `stackman_call` holds the old stack pointer in `x19`. |

`lib/README.md` is about prebuilt archives, not the API or the register sets.
Its platform list omits `riscv64` and the Darwin ABIs that the main README
lists. `CHANGELOG.md` records Windows ARM32 as dropped in 1.0.0.
`platform.h` still selects `switch_arm_msvc.h` when `_M_ARM` is set.

## AArch64

Base non-volatile set, shared by Linux, macOS, and Windows:

- `x19`–`x28`, `x29` (`fp`), `x30` (`lr`)
- low 64 bits of `v8`–`v15` (`d8`–`d15`)

The high 64 bits of `v8`–`v15` are volatile. `x18` is the platform register.
Portable assembly leaves it alone; spilling it and writing it back later does
not make a temporary use of it safe. On Windows user mode it is the TEB pointer
for the whole time user code runs.

`FPCR` control bits are global thread state in AAPCS64. On Windows those same
bits (AHP, DN, FZ, RMode, and the trap-enable bits) are non-volatile. Full SVE
`z8`–`z23` and predicates `p4`–`p15` are callee-saved only for a function that
itself takes or returns scalable arguments. `stackman_switch` is an ordinary C
function, so the base rule applies. `FPMR`, `FFR`, and SME `ZT0` are
caller-saved.

### `switch_aarch64_gcc.S` (Linux `aarch64`, macOS `darwin_arm64`)

`stackman_switch` matches the base set. It allocates 160 bytes and spills
`x19`–`x28`, `x29`, `x30`, and `d8`–`d15`, then loads those slots back. It
never writes `x18`. `stackman_call` spills `x29`, `x30`, and `x19`, and holds
the old stack pointer in `x19`.

`stackman_switch` applies the stack delta to the live `x29` before
`STACKMAN_OP_RESTORE` (`sub x1, sp, x0` / `mov sp, x0` / `sub x29, x29, x1`).
The same three instructions are in `switch_aarch64_gcc.h`, so regenerating
the `.S` from `gen_asm.c` keeps them. The epilogue still reloads the saved
`x29`, so the caller receives the stacked value. The adjustment matters only
during `STACKMAN_OP_RESTORE`. A callback that relocates a stack still has to
rewrite saved frame-pointer slots in the copied bytes.

SME lazy save is not called. AAPCS64 says `setjmp` / `longjmp` call
`__arm_za_disable` so the ZA buffer (`TPIDR2_EL0`) is not left aimed at the
wrong stack. That can wait until a supported target enables SME.

### `switch_arm64_msvc.asm` (Windows `win_arm64`)

The spill set matches the Windows document: `x19`–`x28`, `x29`, `x30`,
`d8`–`d15`, `FPCR`, and `x18`. `x18` is the TEB for the whole time user
code runs. Both routines spill it and leave the live register alone.

`stackman_switch` keeps the context in `x19` and the callback in `x20`.
The first call is `blr x3` after `x20` has been loaded from `x3`, because
the callback clobbers `x3`. The second call is `blr x20`. The `x18` store
and reload stay, so a restored context gets the value it spilled.

`stackman_call` spills `x19` at `[sp, #16]` and keeps the old stack pointer
there. It does not read or write `x18`.

The file adjusts `fp` across the switch, as `switch_aarch64_gcc.S` does for `x29`.
It has no SEH unwind opcodes. The x64 MASM uses `NESTED_ENTRY` and
`.pushreg` / `save_xmm128`. That is an unwinding gap, separate from the
register values. The Windows ARM64 test job compiles this file and does
not run it.

## Other platforms

| File | What it spills | Result |
| --- | --- | --- |
| `switch_x86_64_gcc.S` | `rbx`, `rbp`, `r12`–`r15`, x87 control word, MXCSR | Complete for SysV AMD64. XMM registers are volatile there. The frame pointer is adjusted with the stack delta. |
| `switch_x64_msvc.asm` | `rbx`, `rbp`, `rdi`, `rsi`, `r12`–`r15`, `xmm6`–`xmm15`, MXCSR | Those registers match the Windows x64 ABI. MXCSR is stored at `[rsp+160]`, the pad in the 168-byte xmm frame, and loaded back before that frame is released. `stackman_call` does not spill it. |
| `switch_x86_gcc.S` | `ebx`, `esi`, `edi`, `ebp`, x87 control word | Integer set is complete. There is no callee-saved XMM on i386 SysV. `fstcw` stores the control word at `0x10(%esp)`, the pointer passed to the callback, and `fldcw` loads it after the restore callback. |
| `switch_x86_msvc.asm` | `ebx`, `esi`, `edi`, `ebp`, x87 control word, plus the C wrapper saves `fs:[0]` | Integer set is complete for cdecl. `stackman_switch_raw` stores the control word in a slot below the callee-saved pushes, at the pointer passed to the callback. The C wrapper still restores `fs:[0]`. |
| `switch_arm_gcc.S` | `r4`–`r11`, `lr` | Complete for the soft-float `arm-linux-gnueabi` ABI this tree ships (`.arch armv5t`, `.fpu softvfp`). `d8`–`d15` are not spilled, and `__ARM_PCS_VFP` is a compile error. The live `r11` is adjusted with the stack delta before `STACKMAN_OP_RESTORE`; the epilogue still pops the saved value. |
| `switch_arm_msvc.asm` | `r4`–`r11`, `lr`, `q4`–`q7` (`d8`–`d15`), `FPSCR` | The set matches 32-bit Windows ARM. Alignment does not. `push {r4-r10, fp, lr}` is 9 words, so `sp` is 4-byte aligned for `vpush {q4-q7}`. A doubleword `VPUSH` requires 8-byte alignment. `stackman_call` pushes 3 registers and then `blx`s with `sp` still 4 mod 8. The changelog records this ABI as dropped; the file is still wrong if it is built. |
| `switch_riscv64_gcc.S` | `ra`, `s0`–`s11`, `fs0`–`fs11` | `stackman_switch` is complete for LP64D. `gp` and `tp` are left alone. `fcsr` is thread `fenv` state, not a per-call spill. The optional `s0` frame delta is not applied: `s0` is not a frame chain there, and `stackman_call` uses it to hold the previous stack pointer. |

### RISC-V `stackman_call`

The register spill in `stackman_call` (`ra` and `s0`) is enough for the
registers that routine writes. It used to switch `sp` even when the
requested stack was null, and it passed that new pointer as the third
argument. `test_04` then accepted `passed == stack2`, and `test_05` faulted
in the callback prologue. It now matches the other ports: a null stack is
left in place, and the callback receives the previous stack pointer (`s0`,
captured after the 16-byte frame allocation). The return value stays in
`a0`.

## Suggested fixes

Each item is a local edit. The spill lists that already match the ABI stay
as they are. `stackman_call` only needs a new spill when the fix starts
using a callee-saved register it does not already save.

### AArch64 SME lazy save

On entry to `stackman_switch`, before `sp` changes, call `__arm_za_disable`
through a weak reference when the symbol is present. That is what AAPCS64
requires of `setjmp` / `longjmp`: commit the lazy ZA buffer and clear
`TPIDR2_EL0` before the stack that holds the buffer is abandoned. Do not
spill the ZA array in this file. `FPMR`, `FFR`, and `ZT0` stay
caller-saved. Skip the call on the Windows ARM64 file; this helper is the
AAPCS64 one. This can wait until a supported target actually enables SME.

### Windows ARM32: delete it, or align `sp` before `vpush`

`CHANGELOG.md` says 1.0.0 dropped `win_arm`. The sources are still wired
up: `switch_arm_msvc.asm`, `switch_arm_msvc.h`, the `_M_ARM` branch in
`platform.h`, and the `win_arm` output directories in the VS projects.
Preferred fix: delete that wiring so the file cannot be built.

If the file stays, the entry `sp` is 8-byte aligned and
`push {r4-r10, fp, lr}` subtracts 36 bytes, so `vpush {q4-q7}` runs at
4 mod 8. Push a pad so the count is even, then pad `FPSCR` the same way,
or the `blx` is 4 mod 8 again:

```asm
push {r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}  ; 40 bytes
add  fp, sp, #32                                 ; saved fp is the 9th word
vpush {q4-q7}
vmrs r2, fpscr
push {r2, r3}                                    ; fpscr plus a pad
```

Pop in reverse, with `pc` in place of `lr`. `r3` is a pad; it does not
have to be preserved beyond the pop.

`stackman_call` has the same call-site bug.
`push {r4, fp, lr}` is 12 bytes. Push one more register
(`push {r3, r4, fp, lr}`) and point `fp` at the saved frame pointer with
`add fp, sp, #8`.

## What the current tests cover

`tests/test.c` already exercises a lot of the control path:

- `test_01` checks that `stackman_switch` returns the `STACKMAN_OP_RESTORE`
  result. The no-switch case returns `(void*)1`.
- `test_02` / `test_03` copy a stack out and jump back, which depends on the
  non-volatile image being in the copied bytes.
- `test_04` calls `stackman_call` on a fresh block. The callback writes 256
  bytes on that stack and returns `(void *)(intptr_t)0x1234`. The test
  checks the return value, that a caller local is unchanged, that a local in
  the callback sits on the new block, and that the pointer argument is not
  the stack passed to `stackman_call` and does not fall inside
  `[block, stack)`. The old `STACKMAN_SP_LE(stack2, passed)` check was true
  when `passed == stack2`, which is what the old RISC-V `stackman_call` did.
- `test_05` calls `stackman_call(..., 0)` and checks the same return value
  and caller local. The pointer argument must be non-null and at or below a
  caller local. The old RISC-V code set `sp` to zero and faulted here.
- `test_07` / `test_08` do floating-point and integer work around a no-op
  `stackman_switch`. They hope the compiler keeps values in callee-saved
  registers. If it keeps them in volatile registers or in memory, the test
  passes even when the assembly drops a callee-saved register.

Reading the epilogues, every port leaves the callback result in the ABI
return register (`x0`, `r0`, `a0`, `rax`, `eax`). `test_04` and `test_05`
now guard that.

Linux CI (`.github/workflows/buildcommit.yml`, job `build-linux-gnu`) already
runs `make test` natively for x86-64, with `-m32` for i386, and under
`qemu-arm`, `qemu-aarch64`, and `qemu-riscv64`. macOS runs Intel and Apple
Silicon natively. Windows runs the test binary for x86 and x64 only. The
`arm64` Windows build is compiled and not executed (`matrix.native` is unset).

## Tests

The `stackman_call` contract checks are in `tests/test.c` (`test_04`,
`test_05`). They run on the existing Linux, qemu, and macOS jobs with no new
toolchains. On the first run of those tests, `qemu-riscv64` still printed
`test_03 ok` and then `test_06 ok`: `test_04` and `test_05` were not in the
binary. `STACKMAN_HAVE_CALL` lived inside the `STACKMAN_SWITCH_IMPL` branch
of `switch_riscv64_gcc.h`, so a normal `#include "stackman.h"` never defined
it and `tests/test.c` compiled the calls out. The macro is now set on that
include, with `STACKMAN_STACK_ALIGN` 16, matching the other platform headers.
The following `qemu-riscv64` run failed in `test_04` and `test_05`.
`switch_riscv64_gcc.S` now skips a null stack and passes the previous stack
pointer, which is the contract those tests check.

Do not add a per-register assembly harness yet.

### Assembly clobber harness

A post-return sentinel check is the usual way to prove callee-saved
registers survive. It is a poor fit here.

- A post-return check would have missed the old Windows `x18` bug. That
  routine saved `x18` and wrote it back before `ret`. The damage was the
  live TEB during the callback, so the harness would have seen the restored
  value and passed. Sampling `x18` inside the callback only means something
  on Windows, and the Windows ARM64 job does not run tests.
- It needs a different `.S` or `.asm` for each ABI, kept in lockstep with
  the spill list. The known mistakes above were found by reading those
  lists against the ABI. A harness that encodes the same list will not
  notice a register that both the implementation and the test omit.
- `test_07` and `test_08` already attempt the C version of this and can pass
  for the wrong reason. Extending them will not make the signal much
  stronger.
- SVE, SME, and shadow stacks need a specific qemu CPU model and a compiler
  that emits that state. They are out of scope for a unit test of this
  library's C ABI.

If a harness is added later, limit it to the Linux ABIs the qemu jobs
already run (`sysv_amd64`, `sysv_i386`, `arm32`, `aarch64`, `riscv64`), and
only for the no-op switch:

1. Assembly sets each callee-saved register to a sentinel, after saving the
   real values for its own caller.
2. An assembly callback, not a C callback, overwrites those same registers.
   A C callback is required to preserve them, so it cannot expose a missing
   spill.
3. `stackman_switch` returns the same stack pointer.
4. The harness compares the sentinels, then restores the original registers.

That catches "forgot to spill `d8`" on AArch64. It would not have caught
"`x18` was wrong for the duration of the call", and it does not belong in
the first patch.

### Optional `fenv` check, still in C

`fesetround` before `stackman_switch`, a callback that changes the rounding
mode and returns, then read the mode again. Expect the original mode where
the assembly saves the control word (SysV x86-64 MXCSR and x87 control
word, SysV i386 x87 control word, Windows x64 MXCSR, Windows x86 x87
control word). Expect the callback's mode to stick where the ABI treats
that register as thread state and the assembly does not save it (AAPCS64
`FPCR`, RISC-V `fcsr`). This is ABI-specific and easy to get wrong; land
the `stackman_call` contract tests first.
