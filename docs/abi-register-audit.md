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

The findings below are the audit as it stood when the gaps were found. Each
section keeps that text. **Applied fix** records what was changed afterward.

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

Per-platform comments, checked against the code at audit time:

| Place | Claim | Matches the code |
| --- | --- | --- |
| `switch_x86_64_gcc.h` | `rbx`, `rbp`, `r12`–`r15`, x87 control word, MXCSR | Yes. The comment used to say "mmx"; the instruction is `stmxcsr`. |
| `switch_x86_gcc.h` | `ebx`, `esi`, `edi`, and `ebp` via the frame | Yes for what it saves. It does not mention the x87 control word. |
| `switch_aarch64_gcc.h` | `x19`–`x28`, `x29` via the frame, `d8`–`d15` | Yes for the base AAPCS64 set. Silent on `x18` and SME. |
| `switch_arm_gcc.h` | `r4`–`r11`, and `d8`–`d15` unless Thumb. `FPSCR` is not saved. | The integer list matches `switch_arm_gcc.S`. That `.S` is soft-float and does not save `d8`–`d15`, so `CP_REGISTERS` describes the inline path only. |
| `switch_riscv64_gcc.S` | `s0`–`s11`, `fs0`–`fs11`, `ra` for `stackman_switch` | Yes for that function. `stackman_call` skips a null stack and passes the previous stack pointer. |
| `switch_x64_msvc.asm` | "all registers that the x64 ABI specifies as non-volatile", plus xmm | The GPRs and `xmm6`–`xmm15` match. MXCSR[6:15] is non-volatile on Windows and is not saved, so the comment overclaims. |
| `switch_arm64_msvc.asm` | `x18`–`x28`, `d8`–`d15`, `FPCR` are pushed | The spill matches the comment. The routine then uses `x18` as a scratch register, which the Windows ABI does not allow. |

`lib/README.md` is about prebuilt archives, not the API or the register sets.
Its platform list omits `riscv64` and the Darwin ABIs that the main README
lists. `CHANGELOG.md` records Windows ARM32 as dropped in 1.0.0.
`platform.h` still selects `switch_arm_msvc.h` when `_M_ARM` is set.

**Applied fix.** The i386 header comment now names the x87 control word.
The Windows x64 comment names the non-volatile GPRs, `xmm6`–`xmm15`, and
MXCSR. The ARM32 header rejects `__ARM_PCS_VFP` and says `CP_REGISTERS` is
the inline clobber only. The Windows ARM64 routines no longer write live
`x18`. The `_M_ARM` branch, the Debug|ARM project configurations, and the
prebuilt `lib/win_arm` archive were removed. Windows ARM64 stays. The
`lib/README.md` platform list was left as it was.

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

**Applied fix.** `stackman_switch` applies the stack delta to the live
`x29` before `STACKMAN_OP_RESTORE` (`sub x1, sp, x0` / `mov sp, x0` /
`sub x29, x29, x1`). The same three instructions are in
`switch_aarch64_gcc.h`, so regenerating the `.S` from `gen_asm.c` keeps
them. The epilogue still reloads the saved `x29`. A callback that relocates
a stack still has to rewrite saved frame-pointer slots in the copied bytes.
`__arm_za_disable` is not called. See Roadmap. `FPCR` and `FPSR` are
stored in a 16-byte slot at the stack pointer passed to the callback
and written back after `STACKMAN_OP_RESTORE`. A thread switch saves
both, and two stacks on one thread would otherwise share them.

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

**Applied fix.** `stackman_switch` keeps the context in `x19` and the
callback in `x20`. The first call is `blr x3` after `x20` has been loaded
from `x3`. The second call is `blr x20`. The `x18` store and reload stay,
so a restored context gets the value it spilled. `stackman_call` spills
`x19` at `[sp, #16]` and keeps the old stack pointer there. The SysV
AArch64 file now adjusts `x29` the same way this file adjusts `fp`. SEH
unwind codes were left unchanged. The Windows ARM64 test job compiles this
file and does not run it.

## Other platforms

| File | What it spills | Result |
| --- | --- | --- |
| `switch_x86_64_gcc.S` | `rbx`, `rbp`, `r12`–`r15`, x87 control word, MXCSR | Complete for SysV AMD64. XMM registers are volatile there. The frame pointer is adjusted with the stack delta. |
| `switch_x64_msvc.asm` | `rbx`, `rbp`, `rdi`, `rsi`, `r12`–`r15`, `xmm6`–`xmm15` | Those registers match the Windows x64 ABI. MXCSR[6:15] is non-volatile on Windows and is not spilled, so a changed rounding mode does not travel with the stack. The SysV file does save MXCSR. |
| `switch_x86_gcc.S` | `ebx`, `esi`, `edi`, `ebp` | Integer set is complete. There is no callee-saved XMM on i386 SysV. The x87 control word is callee-saved in that ABI and is not saved. |
| `switch_x86_msvc.asm` | `ebx`, `esi`, `edi`, `ebp`, plus the C wrapper saves `fs:[0]` | Integer set is complete for cdecl. Same x87 control-word gap. |
| `switch_arm_gcc.S` | `r4`–`r11`, `lr` | Complete for the soft-float `arm-linux-gnueabi` ABI this tree ships (`.arch armv5t`, `.fpu softvfp`). `d8`–`d15` are absent. The inline header clobbers them, so a hard-float or softfp build of this `.S` would drop callee-saved VFP state. The live frame pointer is not adjusted. |
| `switch_arm_msvc.asm` | `r4`–`r11`, `lr`, `q4`–`q7` (`d8`–`d15`), `FPSCR` | The set matches 32-bit Windows ARM. Alignment does not. `push {r4-r10, fp, lr}` is 9 words, so `sp` is 4-byte aligned for `vpush {q4-q7}`. A doubleword `VPUSH` requires 8-byte alignment. `stackman_call` pushes 3 registers and then `blx`s with `sp` still 4 mod 8. The changelog records this ABI as dropped; the file is still wrong if it is built. |
| `switch_riscv64_gcc.S` | `ra`, `s0`–`s11`, `fs0`–`fs11` | `stackman_switch` is complete for LP64D. `gp` and `tp` are left alone. `fcsr` is thread `fenv` state, not a per-call spill. |

**Applied fix.**

- `switch_x86_64_gcc.S` was already complete. Nothing was changed.
- `switch_x64_msvc.asm` stores MXCSR at `[rsp+160]`, the pad in the
  168-byte xmm frame, with `stmxcsr` after `alloc_stack` and `ldmxcsr`
  after the GPR pops. `stackman_call` still does not spill MXCSR, because
  it returns to the same caller.
- `switch_x86_gcc.S` and the inline block in `switch_x86_gcc.h` store the
  x87 control word with `fstcw` at `0x10(%esp)`, the pointer passed to the
  callback, and reload it with `fldcw` after the restore callback. `fstcw`
  is the same instruction `switch_x86_64_gcc.S` uses. XMM registers stay
  unsaved.
- `switch_x86_msvc.asm` stores the control word in `stackman_switch_raw`,
  in a slot allocated with `ebx`, `esi`, and `edi`, at the pointer passed
  to the callback. The C wrapper still restores `fs:[0]`.
- `switch_arm_gcc.h` is a compile error when `__ARM_PCS_VFP` is set.
  `d8`–`d15` were not added. `CP_REGISTERS` is marked as the inline-only
  clobber. An `r11` stack delta was added and then removed.
  `stackman_switch` is compiled with `omit-frame-pointer`, and the
  prologue pushes `fp` without setting it, so live `r11` is not this
  frame. The epilogue pops the saved value. An inline build can also
  keep the callback or the new stack pointer in `r11` across that asm.
  AArch64 still adjusts `x29`, which the prologue sets with
  `mov x29, sp`.
- `__ARM_PCS_VFP` selects `armhf` (`switch_armhf_gcc.S`), which spills
  `d8`–`d15`. `FPSCR` is still not spilled. The soft-float file was
  left unchanged.
- Windows ARM32 was removed: `switch_arm_msvc.asm`, `switch_arm_msvc.h`,
  the `_M_ARM` branch in `platform.h`, the Debug|ARM configurations in
  vs2017, vs2019, and vs2022, and `lib/win_arm/stackman.lib`. The
  alignment alternative was not used. Windows ARM64 stays.
- The RISC-V register spill was already complete. The optional `s0` frame
  delta was left unchanged: `s0` is not a frame chain in `stackman_switch`,
  and `stackman_call` uses it to hold the previous stack pointer.

### RISC-V `stackman_call`

The register spill in `stackman_call` (`ra` and `s0`) is enough for the
registers that routine writes. It used to switch `sp` even when the
requested stack was null, and it passed that new pointer as the third
argument. `test_04` then accepted `passed == stack2`, and `test_05` faulted
in the callback prologue.

**Applied fix.** It now matches the other ports: a null stack is left in
place, and the callback receives the previous stack pointer (`s0`, captured
after the 16-byte frame allocation). The return value stays in `a0`.

## Suggested fixes

Each item is a local edit. The spill lists that already match the ABI stay
as they are. `stackman_call` only needs a new spill when the fix starts
using a callee-saved register it does not already save.

The steps under each heading are the ones the audit proposed. The applied
fix at the end of the heading says what landed.

### Windows ARM64: stop writing `x18`

File: `stackman/platforms/switch_arm64_msvc.asm`.

`stackman_switch` already spills `x19`–`x28` before it assigns them. Keep
the context in `x19`. Keep the callback in `x20` instead of `x18`, and make
the second call `blr x20`. The first call can stay `blr x3` only if `x20`
was loaded from `x3` first; the callback clobbers `x3`. Leave the `x18`
store and reload in place so a restored context gets the value it spilled,
and do not write `x18` between them.

`stackman_call` should spill `x19` in the slot that now holds `x18`
(`str`/`ldr` at `[sp, #16]`), keep the old stack pointer in `x19`, and
delete every `x18` access. The 32-byte frame is already large enough.

Unwind data is a separate change. Describe the prologue with the ARM64
counterparts of the `NESTED_ENTRY` / `save_xmm128` macros in
`switch_x64_msvc.asm`, one unwind code per store, so `RtlUnwind` can find
`x19`–`x30`, `d8`–`d15`, and the `FPCR` slot. The Windows ARM64 test job
compiles this file and does not run it, so the `x18` fix needs a Windows
ARM64 machine or an emulator before it can be considered tested.

**Applied fix.** The `x18` change was made as written above. Unwind codes
were not added. `FPSR` is stored at `[sp, #176]` next to the existing
`FPCR` spill. The frame is 192 bytes. `stackman_call` still does not
spill either register.

### AArch64 SysV and Apple: keep `x29` attached to the new stack

File: `stackman/platforms/switch_aarch64_gcc.S`, in `stackman_switch`,
immediately after the first `blr` and before `mov w1, 1`.

`x0` is the new stack pointer. `x19` is the callback and `x20` is the
context. `x1` is free until it becomes the opcode:

```asm
sub x1, sp, x0
mov sp, x0
sub x29, x29, x1
```

That is the same delta the Windows ARM64 file applies to `fp`. The epilogue
still reloads `x29` from the stack, so the caller receives the saved value,
not this adjusted one. The adjustment matters only while
`STACKMAN_OP_RESTORE` runs. A callback that relocates a stack still has to
rewrite saved frame-pointer slots in the copied bytes; this edit does not do
that.

The checked-in `.S` is what default builds use (`STACKMAN_INLINE_ASM` is
unset). The inline sequence in `switch_aarch64_gcc.h` is the generator. Put
the same three instructions in that header if the `.S` is regenerated from
`gen_asm.c`, or the next regeneration will drop the delta.

The same three-instruction pattern applies to `switch_arm_gcc.S`
(`sub r1, sp, r0` / `mov sp, r0` / `sub r11, r11, r1`) and to
`switch_riscv64_gcc.S` (`sub t0, sp, a0` before `mv sp, a0`, then add that
delta to `s0` only when `s0` is the live frame pointer). On RISC-V `s0` is
just a saved register in `stackman_switch`, not a frame chain, so the
integer spill is already complete and the delta is optional. `stackman_call`
uses `s0` to hold the previous stack pointer across the callback, so do not
adjust it there.

**Applied fix.** The `x29` delta is in `switch_aarch64_gcc.S` and in
`switch_aarch64_gcc.h`. The `r11` delta was added to `switch_arm_gcc.S`
and `switch_arm_gcc.h`, then removed: that function omits the frame
pointer, so the delta does not land on this frame. The RISC-V `s0` delta
was left unchanged.

### AArch64 SME lazy save

On entry to `stackman_switch`, before `sp` changes, call `__arm_za_disable`
through a weak reference when the symbol is present. That is what AAPCS64
requires of `setjmp` / `longjmp`: commit the lazy ZA buffer and clear
`TPIDR2_EL0` before the stack that holds the buffer is abandoned. Do not
spill the ZA array in this file. `FPMR`, `FFR`, and `ZT0` stay
caller-saved. Skip the call on the Windows ARM64 file; this helper is the
AAPCS64 one. This can wait until a supported target actually enables SME.

**Applied fix.** None. `__arm_za_disable` is the wrong tool for this
switch. See Roadmap.

### Windows x64: spill MXCSR with the xmm frame

File: `stackman/platforms/switch_x64_msvc.asm`, `stackman_switch` only.

`alloc_stack 168` covers `xmm6`–`xmm15` (160 bytes) plus an 8-byte pad at
`[rsp+160]`. After `alloc_stack` and before the `push_reg`s, `stmxcsr`
that dword. After the epilogue pops the GPRs and before `add rsp, 168`,
`ldmxcsr` the same slot. Restoring the whole register also restores the
volatile status flags, which is what `switch_x86_64_gcc.S` already does.
`stackman_call` does not need this: it returns to the same caller, and a
conforming callback restores MXCSR itself.

Narrow the comment that says every non-volatile register is saved, once
MXCSR is included, or the comment stays ahead of the code.

**Applied fix.** MXCSR is spilled and restored at `[rsp+160]` as described.
The comment now names the non-volatile GPRs, `xmm6`–`xmm15`, and MXCSR.

### i386: spill the x87 control word

The integer spills are complete. The gap is `fnstcw` / `fldcw` around the
two callbacks in `stackman_switch`, stored inside the frame that the
callback is asked to copy.

- `switch_x86_gcc.S` and the inline block in `switch_x86_gcc.h` must
  change together. The `.S` is a copy of the header's asm string. The
  `subl $0x1c, %esp` frame has unused bytes at `0x10(%esp)`, `0x14(%esp)`,
  and `0x18(%esp)`, and `0x10(%esp)` is the pointer passed to the callback.
  `fnstcw` a word at that pointer before the first `call`, and `fldcw` it
  after the second `call`, before `addl $0x1c, %esp`. After
  `leal -0x10(%eax), %esp` the same word is at `0x10(%esp)` on the new
  stack. Regenerating with `gcc -S -m32 -fcf-protection=none` from
  `gen_asm.c` is the project's usual way to refresh the `.S`.
- `switch_x86_msvc.asm`: `fnstcw` / `fldcw` in `stackman_switch_raw`, in a
  slot allocated with `ebx` / `esi` / `edi`, so the word is on the stack
  image rather than in the C wrapper's `seh_state` local. The wrapper
  already restores `fs:[0]` and should keep doing that.

XMM registers stay unsaved: they are volatile for SysV i386 and for
Windows cdecl. `__vectorcall` is not the convention this library uses.

**Applied fix.** Both gcc files and `stackman_switch_raw` store and reload
the control word in the frame the callback is asked to copy. The
instruction is `fstcw` / `fldcw`, matching `switch_x86_64_gcc.S`. The
wrapper still restores `fs:[0]`.

### ARM32 soft-float: refuse a hard-float build

`switch_arm_gcc.S` is `.arch armv5t` and `.fpu softvfp`, which matches
`arm-linux-gnueabi`. Do not add `d8`–`d15` until there is an `armhf` ABI.
Do stop a hard-float compile from silently using this file. In
`switch_arm_gcc.h`, `#error` when `__ARM_PCS_VFP` is set, with a line that
says the checked-in assembly does not spill `d8`–`d15`. Leave
`CP_REGISTERS` as the inline-only clobber, or drop it, so it cannot be read
as a description of `switch_arm_gcc.S`.

The live `r11` adjustment in the AArch64 section above is optional here.
The shipped integer set (`r4`–`r11`, `lr`) needs no new stores.

**Applied fix.** `__ARM_PCS_VFP` is a compile error, and `CP_REGISTERS` is
commented as inline-only. The optional `r11` adjustment was applied and
then removed. See the frame-pointer applied fix above. `d8`–`d15` were
not added to this file. `__ARM_PCS_VFP` selects `armhf`
(`switch_armhf_gcc.S` spills `d8`–`d15`; `FPSCR` is still not spilled).

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

**Applied fix.** The preferred option was taken. The sources, the
`_M_ARM` branch, the Debug|ARM configurations in vs2017, vs2019, and
vs2022, and `lib/win_arm/stackman.lib` were deleted. The alignment
alternative was not used.

## Roadmap

SME lazy save is still open. The ARM32 hard-float port is `armhf`.

### ARM32 hard-float

The soft-float file `switch_arm_gcc.S` is unchanged (`arm-linux-gnueabi`,
`.arch armv5t`, `.fpu softvfp`) and does not spill `d8`–`d15`.
`__ARM_PCS_VFP` is still a compile error in that header, so a hard-float
build cannot use that file by mistake.

`switch_armhf_gcc.h` is the hard-float port. `platform.h` selects it,
and the ABI name `armhf`, when `__ARM_PCS_VFP` is set. `stackman_switch`
spills `d8`–`d15` and the whole `FPSCR`.

Softfp (`-mfloat-abi=softfp`) is not a supported platform, and it is not
clear how widely it is used. It does not define `__ARM_PCS_VFP`, so the
build selects `arm32` and runs. That assembly does not preserve `FPSCR`.
The compiler can still use `d8`–`d15` while building `arm32`. Do not add
an `FPSCR` spill to the soft-float file to cover softfp: that file is
`.fpu softvfp`, and `vmrs fpscr` is an undefined instruction without VFP.

### AArch64 SME lazy save

SME (Scalable Matrix Extension) adds ZA, a matrix accumulator that is much
larger than the general-purpose and SIMD registers. A function that has ZA
live can avoid copying it on every call by leaving it dormant:
`TPIDR2_EL0` points at a save buffer on that function's stack, and a
normal callee leaves the buffer alone. `stackman_switch` can abandon that
stack, so the pointer would dangle. A later use of ZA would write through
it.

`__arm_za_disable` is the helper `setjmp` uses. If the thread has SME, it
writes ZA into the buffer of the function that called `setjmp`, clears
`TPIDR2_EL0`, and turns ZA off. The compiler at that same call site turns
ZA back on and copies the buffer back. The address is a local in that
frame, so clearing `TPIDR2_EL0` does not lose it. `longjmp` returns to
that same frame, and the reload puts back the snapshot from `setjmp`.

That is a single call site resuming itself. `stackman_switch` resumes
whichever stack the callback selects. The incoming activation's ZA is not
in the callee-saved spill, and its call site was not compiled to reload a
lazy-save buffer. Calling the helper on the way out would commit the
outgoing caller's ZA and turn ZA off, and nothing in the switch would
install the incoming caller's ZA.

Leave the helper uncalled. A stack that uses SME would need its own ZA
image saved and restored with that stack, which this port does not do.
`stackman_call` returns to the same caller and does not need it either.

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

**Applied fix.** The `x18` bug itself was fixed, as described above. No
harness was added. A post-return sentinel would still miss a bug of that
shape.

### Optional `fenv` check, still in C

`fesetround` before `stackman_switch`, a callback that changes the rounding
mode and returns, then read the mode again. Expect the original mode where
the assembly saves the control word (SysV x86-64). Expect the callback's
mode to stick where the ABI treats that register as thread state and the
assembly does not save it (AAPCS64 `FPCR`, RISC-V `fcsr`). This is
ABI-specific and easy to get wrong; land the `stackman_call` contract tests
first.

**Applied fix.** No `fenv` test was added. After the spills above, the
original mode should also come back on SysV i386 (x87 control word),
Windows x86 (x87 control word), Windows x64 (MXCSR), and the 64-bit ARM
ABIs (`FPCR`, with `FPSR` restored alongside it). RISC-V `fcsr` is still
not spilled.
