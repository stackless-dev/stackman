# Platform ABI and register spills

Current contract for `stackman_switch` and `stackman_call`. The assembly
and inline headers are the source of truth. This file is the map.

`docs/abi-register-audit.md` is the audit from when the gaps were found
and fixed. Do not add later ports or spills to that file.

## Switching

`stackman_switch` may resume a different caller. It spills the
non-volatile set for that ABI, calls the callback with
`STACKMAN_OP_SAVE`, installs the stack pointer that callback returns,
calls it again with `STACKMAN_OP_RESTORE`, restores the saved state,
and returns the second result.

`stackman_call` returns to the same caller. A null stack is left
unchanged. The pointer passed to the callback is the previous stack
pointer. The routine preserves only registers it modifies.

Volatile registers may change across either call.

A floating-point control or status register is spilled in
`stackman_switch` when a thread switch saves it. The whole register is
stored, including caller-saved flag bits. The store is in the frame
before that stack pointer is published to the callback. The reload is
after `STACKMAN_OP_RESTORE` and before the epilogue pops callee-saved
registers. `stackman_call` does not touch these registers.
`tests/test.c` `test_09` checks the caller's rounding mode. It comes
back on every ABI below except soft-float `arm32`, where the test
expects the callback's mode to remain.

`STACKMAN_SP_ALIGN` follows `STACKMAN_STACK_ALIGN` below.

## Platforms

`platform.h` selects the source. `win_aarch64` is accepted and
normalized to `win_arm64`. FreeBSD, OpenBSD, and NetBSD use the
`sysv_amd64` sources. Windows ARM32 (`win_arm`) was removed in 1.0.0.

| ABI | Source | `STACKMAN_STACK_ALIGN` |
| --- | --- | --- |
| `sysv_i386` | `switch_x86_gcc` | 16 |
| `sysv_amd64`, `darwin_x86_64` | `switch_x86_64_gcc` | 16 |
| `arm32` | `switch_arm_gcc` | 8 |
| `armhf` | `switch_armhf_gcc` | 8 |
| `aarch64`, `darwin_arm64` | `switch_aarch64_gcc` | 16 |
| `riscv64` | `switch_riscv64_gcc` | 16 |
| `win_x86` | `switch_x86_msvc` | 4 |
| `win_x64` | `switch_x64_msvc` | 16 |
| `win_arm64` | `switch_arm64_msvc` | 16 |

### `sysv_i386`

`stackman_switch` spills `ebx`, `esi`, `edi`, `ebp`, and the x87
control word (`fstcw` / `fldcw`). It does not spill MXCSR. There is no
callee-saved XMM register in this ABI.

`stackman_call` does not spill the x87 control word.

### `sysv_amd64` and `darwin_x86_64`

`stackman_switch` spills `rbx`, `rbp`, `r12`–`r15`, the x87 control
word (`fstcw` / `fldcw`), and MXCSR (`stmxcsr` / `ldmxcsr`). XMM
registers are volatile. `rbp` is adjusted by the stack delta when the
callback installs a new stack.

`stackman_call` does not spill the x87 control word or MXCSR.

### `arm32`

Soft-float AAPCS, `arm-linux-gnueabi`. The assembly is `.arch armv5t`
and `.fpu softvfp`. `__ARM_PCS_VFP` is a compile error in this header.

`stackman_switch` spills `r4`–`r11` and `lr`. It does not spill
`d8`–`d15` or `FPSCR`. `r11` is not a frame chain here:
`omit-frame-pointer` is set, the prologue pushes `r11` without writing
it, and the epilogue pops the saved value. Do not adjust `r11`.

`stackman_call` does not spill `d8`–`d15` or `FPSCR`.

Softfp (`-mfloat-abi=softfp`) is not a supported platform. It does not
define `__ARM_PCS_VFP`, so it selects `arm32` and runs, and this
assembly does not preserve `FPSCR`. Do not add `vmrs fpscr` to
`switch_arm_gcc.S`. That instruction is undefined without VFP. The
compiler can still use `d8`–`d15` while building `arm32`.

### `armhf`

Hard-float AAPCS-VFP, `arm-linux-gnueabihf`. `__ARM_PCS_VFP` selects
this ABI. It does not link with `arm32`.

`stackman_switch` spills `r4`–`r11`, `lr`, `d8`–`d15`, and the whole
`FPSCR`. The published stack pointer is the 8-byte `FPSCR` slot, below
the `d8`–`d15` spill. That slot is written back before those registers
are popped. `r3` in the integer push is padding. `r11` is not adjusted,
for the same reason as `arm32`.

`stackman_call` does not spill `d8`–`d15` or `FPSCR`.

### `aarch64` and `darwin_arm64`

`stackman_switch` spills `x19`–`x28`, `x29`, `x30`, and the low 64 bits
of `v8`–`v15` (`d8`–`d15`). The high 64 bits of `v8`–`v15` are
caller-saved and are not spilled. `x18` is left alone.

`FPCR` and `FPSR` are stored in a 16-byte slot at the pointer passed to
the callback and written back after `STACKMAN_OP_RESTORE`. `x29` is
adjusted by the stack delta. The prologue sets `x29` from `sp`, so the
delta is computed from the lowered stack pointer. Do not list `x29` as
an asm clobber.

`stackman_call` does not spill `FPCR`, `FPSR`, or `d8`–`d15`.

SME ZA is not preserved. Do not call `__arm_za_disable`.

### `win_arm64`

`stackman_switch` spills `x18`–`x28`, `x29`, `x30`, `d8`–`d15`, `FPCR`,
and `FPSR`. The frame is 192 bytes. `FPCR` is stored with `x18` at
`[sp, #16]`. `FPSR` is the 8-byte slot at `[sp, #176]`, next to `d15`,
with 8 bytes of padding at `[sp, #184]`. The body does not use `x18`
as a scratch. The epilogue reloads the spilled value. `x18` is the TEB
for the whole time user code runs. The frame pointer is adjusted by
the stack delta. No `.seh_*` unwind codes are emitted. The high 64
bits of `v8`–`v15` are not spilled.

`stackman_call` uses a 32-byte frame. It does not touch `x18`, `FPCR`,
or `FPSR`.

SME ZA is not preserved. Do not call the AAPCS64 `__arm_za_disable`
helper on Windows.

### `riscv64`

LP64D. `stackman_switch` spills `ra`, `s0`–`s11`, `fs0`–`fs11`, and the
whole `fcsr` (`frm` and the accrued exception flags). The frame is 208
bytes. `fcsr` uses the existing 8-byte pad at the published stack
pointer, below `fs11`. `gp` and `tp` are left alone. `s0` is not a frame
chain in `stackman_switch`. Do not adjust it.

`stackman_call` spills `ra` and `s0`. `s0` holds the previous stack
pointer. A null stack is left unchanged. It does not spill `fcsr`.

Vector CSRs (`vcsr`, `vl`, `vtype`, `vxrm`, `vxsat`) are not spilled.

### `win_x86`

`stackman_switch_raw` spills `ebx`, `esi`, `edi`, `ebp`, the x87
control word (`fstcw` / `fldcw`), and MXCSR (`stmxcsr` / `ldmxcsr`).
The published pointer is that 8-byte slot: the control word at `[esp]`,
MXCSR at `[esp+4]`. Both are spilled so older compilers, which emit
x87, and current MSVC, which emits SSE2, each keep their rounding mode.
The C wrapper around the raw routine saves and restores `fs:[0]`, the
SEH chain.

`stackman_call` does not spill the x87 control word or MXCSR. The SEH
save is only on `stackman_switch`.

### `win_x64`

`stackman_switch` spills `rbx`, `rbp`, `rdi`, `rsi`, `r12`–`r15`,
`xmm6`–`xmm15`, and MXCSR. MXCSR sits in the 8-byte pad at the end of
the 168-byte XMM area.

`stackman_call` does not spill MXCSR or `xmm6`–`xmm15`.
