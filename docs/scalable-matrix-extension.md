# Scalable Matrix Extension

SME is the Scalable Matrix Extension. It is an optional AArch64 feature,
`FEAT_SME`, published by Arm in 2021 as part of the Armv9-A architecture.
This note is how `stackman_switch` keeps its ZA state on `aarch64` and
`darwin_arm64`.
The register spills for the other ABIs stay in `platform-abi.md`.

## Where it comes from

SME is a CPU feature, not an operating-system feature and not a baseline
part of AArch64. A core either implements `FEAT_SME` or it does not. Armv8-A
machines do not have it. Plenty of Armv9-A machines do not have it either.
Cores that do include Apple M4 and later, and some Neoverse and Cortex
parts from that generation. The streaming vector length is chosen by the
implementation. ZA is one square matrix of that length: from 256 bytes at
the shortest length up to 64 KB at the longest.

Software sees it through the Arm Procedure Call Standard for the 64-bit
Architecture (AAPCS64). That document defines the ZA lazy-save scheme and
the runtime helpers (`__arm_sme_state`, `__arm_za_disable`,
`__arm_tpidr2_restore`, and the others). The helpers live in the compiler
runtime, libgcc or compiler-rt, on toolchains that implement the scheme.
They ask the platform whether this thread may use SME, and they return
without touching ZA when it may not. A library built for plain `armv8-a`
can call them.

Linux and macOS on AArch64 use that AAPCS64 runtime. In this tree those
are the `aarch64` and `darwin_arm64` ABIs, both built from
`switch_aarch64_gcc.S`. Windows on ARM64 does not provide these helpers
and does not use this lazy-save scheme. `win_arm64` is out of scope here.
`stackman_call` returns to the same caller, so the caller's buffer stays
on the live stack and this scheme is not required there.

SVE, the Scalable Vector Extension, widens the vector registers. Advanced
SIMD is the 128-bit `v0`–`v31` file. ZA is neither of those. It is a
separate matrix accumulator with its own enable bit.

## The three modes

ZA is in exactly one of these modes. The names are the AAPCS64 ones.
`PSTATE.ZA` is the enable bit. `TPIDR2_EL0` is a system register that
either holds null or points at a 16-byte TPIDR2 block on some function's
stack. Byte 0 of that block is `za_save_buffer`, the address of the rows.
Bytes 8–9 are `num_za_save_slices`. The matrix is not stored at the
pointer itself.

**Off.** `PSTATE.ZA` is 0 and `TPIDR2_EL0` is null. The matrix is disabled
and holds nothing that matters. `__arm_za_disable` leaves this mode
behind. This is the disabled state.

**Active.** `PSTATE.ZA` is 1 and `TPIDR2_EL0` is null. The matrix is on
and the bytes sit in the hardware register. The function that is running
is using ZA directly. There is no buffer waiting to receive a copy.

**Dormant.** `PSTATE.ZA` is 1 and `TPIDR2_EL0` points at a TPIDR2 block.
The matrix is still on, and the bytes are still in the hardware register.
The block says where to copy them if some other function needs the
register. The buffer is stale until that copy happens. Dormant exists
because the matrix is large. A normal call does not copy it. The owner
sets the pointer, makes the call, and on return either clears the pointer
(the callee never needed ZA, so the register still holds the bytes) or
finds the pointer already cleared (the callee committed a copy, and the
owner loads the buffer back).

Active and dormant both have the matrix turned on. The difference is the
pointer. Off is the only mode in which the matrix is disabled.

`stackman_switch` is an ordinary C function. The procedure call standard
classifies every such function as private-ZA. On entry, ZA is off or
dormant. It is not active. A caller that was using ZA switches from
active to dormant before the call, by pointing `TPIDR2_EL0` at a block in
its own frame. At the first instruction of `stackman_switch` the bytes are
still in the hardware register.

## How a switch keeps that state

The goal is to put back the mode `stackman_switch` found on entry, for
whichever stack is about to run. Off stays off. Dormant becomes dormant
again, with the same block pointer and the same bytes in the register.

Do this with the AAPCS64 helpers, not with `__arm_sme_save` and
`__arm_sme_restore`. That second pair snapshots active ZA into a private
buffer and does nothing when `TPIDR2_EL0` is already set. The mode on
entry is dormant or off, so the pair would not see it.

Saving the outgoing stack happens before the `STACKMAN_OP_SAVE` callback.
That callback is allowed to copy or abandon the stack, and a dormant
`TPIDR2_EL0` points into it.

1. Call `__arm_sme_state`. It is safe on a machine without SME. The
   TPIDR2 value comes back in `x1`. Do not read `TPIDR2_EL0` with `mrs`.
   That register traps when the thread has no access to it.
2. If the pointer is null, the mode is off. Store a null word in this
   stack's spill image.
3. If the pointer is non-null, the mode is dormant. Keep the pointer,
   then call `__arm_za_disable`. The helper copies the live rows into
   `za_save_buffer`, clears `TPIDR2_EL0`, and turns ZA off. Store the
   saved pointer in the spill image.

The spill slot has to sit at or above the stack pointer that the callback
sees, or a copy of the stack will not carry it. The published pointer
stays the `FPCR`/`FPSR` slot. The block pointer is the next 8 bytes, with
8 bytes of padding so the slot stays 16-byte aligned:

| Offset from the published stack pointer | Contents |
| --- | --- |
| 0 | `FPCR` |
| 8 | `FPSR` |
| 16 | saved TPIDR2 block pointer, or 0 |
| 24 | 0 |

Restoring the incoming stack happens after its stack pointer is installed
and before the `STACKMAN_OP_RESTORE` callback. Code on that stack then
sees the mode its own `stackman_switch` entry had.

1. Load the saved pointer from that stack's slot.
2. A null pointer means off. Leave ZA disabled and leave `TPIDR2_EL0`
   null.
3. A non-null pointer means dormant. `smstart za` turns the matrix on
   without changing streaming mode (`PSTATE.SM` is already 0 on entry to
   this non-streaming function). Call `__arm_tpidr2_restore` with the
   saved block pointer. That helper requires `TPIDR2_EL0` to be null and
   ZA to be on, and it only copies the rows. Then write the pointer back
   to `TPIDR2_EL0`. The mode is dormant again.

The same steps run when the callback does not switch. Entry was dormant,
the save committed it, and the restore puts dormant back before
`stackman_switch` returns. The caller's compiler then sees its own block
pointer still in `TPIDR2_EL0` and keeps the register contents.

Link the three helpers as weak symbols. A toolchain that does not ship
them resolves the addresses to zero. In that case store a null pointer
and skip both calls, which is the behaviour this tree has today. If
`__arm_tpidr2_restore` is missing, do not call `__arm_za_disable` either:
committing without a way to load the buffer back would drop the rows.
Windows ARM64 does not get these calls.

Two limits belong with the mechanism. The saved pointer and
`za_save_buffer` are absolute addresses. A callback that copies a stack
to a new base has to relocate them, the same way it already has to
relocate saved frame pointers. `ZT0`, the SME2 lookup table, is
caller-saved and is not part of this state.
