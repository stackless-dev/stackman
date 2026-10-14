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

Two gaps that are not missing base registers:

- After `mov sp, x0` the live `x29` is left pointing at the old stack. The
  Windows ARM64 and SysV x86-64 routines add the same delta to the frame
  pointer. The epilogue still reloads the saved `x29`, so the caller receives
  the stacked value. The frame chain is wrong only during
  `STACKMAN_OP_RESTORE`.
- A stack switch is an asynchronous transfer of control. AAPCS64 says
  `setjmp` / `longjmp` call `__arm_za_disable` so the SME lazy-save buffer
  (`TPIDR2_EL0`) is not left aimed at the wrong stack. This file does not.

### `switch_arm64_msvc.asm` (Windows `win_arm64`)

The spill set matches the Windows document: `x19`–`x28`, `x29`, `x30`,
`d8`–`d15`, `FPCR`, and `x18`. The bug is the live value of `x18` during the
callbacks.

`stackman_switch` stores the callback in `x18` and calls through it:

```asm
mov x19, x0
mov x18, x3
blr x3          ; first callback, x18 is no longer the TEB
...
blr x18         ; second callback
```

`stackman_call` parks the old stack pointer in `x18` across `blr x3`. TLS,
the stack limit, `__chkstk`, and any Win32 call read the TEB through `x18`.
The epilogue writes the saved value back, so the original caller is repaired
only when the callback returns.

Fix: keep `x18` unchanged. `stackman_switch` already spills `x19`–`x28`; use
one of those for the callback. `stackman_call` should spill some other
non-volatile (for example `x19`) and keep the old stack pointer there.

The Windows file adjusts `fp` across the switch, which the SysV file does not.
It has no SEH unwind opcodes. The x64 MASM uses `NESTED_ENTRY` and
`.pushreg` / `save_xmm128`. That is an unwinding gap, separate from the
register values.

## Other platforms

| File | What it spills | Result |
| --- | --- | --- |
| `switch_x86_64_gcc.S` | `rbx`, `rbp`, `r12`–`r15`, x87 control word, MXCSR | Complete for SysV AMD64. XMM registers are volatile there. The frame pointer is adjusted with the stack delta. |
| `switch_x64_msvc.asm` | `rbx`, `rbp`, `rdi`, `rsi`, `r12`–`r15`, `xmm6`–`xmm15` | Those registers match the Windows x64 ABI. MXCSR[6:15] is non-volatile on Windows and is not spilled, so a changed rounding mode does not travel with the stack. The SysV file does save MXCSR. |
| `switch_x86_gcc.S` | `ebx`, `esi`, `edi`, `ebp` | Integer set is complete. There is no callee-saved XMM on i386 SysV. The x87 control word is callee-saved in that ABI and is not saved. |
| `switch_x86_msvc.asm` | `ebx`, `esi`, `edi`, `ebp`, plus the C wrapper saves `fs:[0]` | Integer set is complete for cdecl. Same x87 control-word gap. |
| `switch_arm_gcc.S` | `r4`–`r11`, `lr` | Complete for the soft-float `arm-linux-gnueabi` ABI this tree ships (`.arch armv5t`, `.fpu softvfp`). `d8`–`d15` are absent. The inline header clobbers them, so a hard-float or softfp build of this `.S` would drop callee-saved VFP state. `armhf` is not a configured ABI. The live frame pointer is not adjusted. |
| `switch_arm_msvc.asm` | `r4`–`r11`, `lr`, `q4`–`q7` (`d8`–`d15`), `FPSCR` | The set matches 32-bit Windows ARM. Alignment does not. `push {r4-r10, fp, lr}` is 9 words, so `sp` is 4-byte aligned for `vpush {q4-q7}`. A doubleword `VPUSH` requires 8-byte alignment. `stackman_call` pushes 3 registers and then `blx`s with `sp` still 4 mod 8. The changelog records this ABI as dropped; the file is still wrong if it is built. |
| `switch_riscv64_gcc.S` | `ra`, `s0`–`s11`, `fs0`–`fs11` | `stackman_switch` is complete for LP64D. `gp` and `tp` are left alone. `fcsr` is thread `fenv` state, not a per-call spill. |

### RISC-V `stackman_call` control bugs

The register spill in `stackman_call` (`ra` and `s0`) is enough for the
registers that routine writes. The control flow is not.

```asm
mv t0, a0
mv t1, a2
mv sp, a2       ; runs even when a2 is 0
...
mv a2, t1       ; third argument is the new stack, not the old one
jalr t0
```

Every other port skips a null stack pointer and passes the previous stack
pointer. The function comment on `stackman_call` says the third argument is
the previous stack pointer (`stackman/stackman_switch.h`). The
`STACKMAN_OP_CALL` enum comment says the opposite: the pointer that was
provided to `stackman_call`. The implementations other than RISC-V follow the
function comment.

A null stack on RISC-V sets `sp` to zero and then calls the callback. That
faults inside the callback prologue. The return value itself is left in `a0`
and would survive the epilogue on a non-null stack.

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
  when `passed == stack2`, so the RISC-V bug satisfied it.
- `test_05` calls `stackman_call(..., 0)` and checks the same return value
  and caller local. The pointer argument must be non-null and at or below a
  caller local. On RISC-V this still faults: `sp` is set to zero before the
  callback runs, so the failure is a crash rather than an assertion.
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
toolchains. `qemu-riscv64` is expected to fail until `switch_riscv64_gcc.S`
skips a null stack and passes the previous stack pointer: `test_04` asserts,
and `test_05` faults. Fix the `STACKMAN_OP_CALL` enum comment in the same
change, so both comments say the third argument is the previous stack pointer.

Do not add a per-register assembly harness yet.

### Assembly clobber harness

A post-return sentinel check is the usual way to prove callee-saved
registers survive. It is a poor fit here.

- It misses the Windows `x18` bug. That routine saves `x18` and writes it
  back before `ret`. The damage is the live TEB during the callback. The
  harness would see the restored value and pass. Sampling `x18` inside the
  callback only means something on Windows, and the Windows ARM64 job does
  not run tests.
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

That catches "forgot to spill `d8`" on AArch64. It still does not catch
"`x18` was wrong for the duration of the call", and it does not belong in
the first patch.

### Optional `fenv` check, still in C

`fesetround` before `stackman_switch`, a callback that changes the rounding
mode and returns, then read the mode again. Expect the original mode where
the assembly saves the control word (SysV x86-64). Expect the callback's
mode to stick where the ABI treats that register as thread state and the
assembly does not save it (AAPCS64 `FPCR`, RISC-V `fcsr`). This is
ABI-specific and easy to get wrong; land the `stackman_call` contract tests
first.
