# Agent notes for stackman

Low-level C library for stack switching. Two functions, no dependencies.
Read the sources. This file is a map, not a second copy of them. The
version is `STACKMAN_VERSION` in `stackman/stackman.h`.

## What the library does

`stackman_switch` saves callee-saved state, calls the callback with
`STACKMAN_OP_SAVE`, installs the stack pointer that callback returns,
calls it again with `STACKMAN_OP_RESTORE`, then restores the saved state
and returns that second result. It may resume a different caller, so the
spill is the whole non-volatile set for that ABI.

`stackman_call` runs the callback on another stack and returns to the
same caller. A null stack is left unchanged. The pointer passed to the
callback is the previous stack pointer. It only needs to preserve
registers it modifies.

User code includes `stackman.h`. The library implementation includes
`stackman_impl.h` once. `platforms/platform.h` selects
`switch_<abi>_<compiler>`.

`STACKMAN_HAVE_CALL` and `STACKMAN_STACK_ALIGN` must be visible from a
normal `#include "stackman.h"`. If they sit under
`#ifdef STACKMAN_SWITCH_IMPL`, `tests/test.c` compiles `stackman_call`
out and CI looks green.

Align a stack with `STACKMAN_SP_ALIGN`. It follows `STACKMAN_STACK_ALIGN`
and the stack direction.

## Platforms

Linux: `sysv_i386`, `sysv_amd64`, `arm32` (soft-float AAPCS), `armhf`
(AAPCS-VFP), `aarch64`, `riscv64` (LP64D). macOS: `darwin_x86_64`,
`darwin_arm64`. Windows:
`win_x86`, `win_x64`, `win_arm64`. `win_aarch64` is accepted and
normalized to `win_arm64`. FreeBSD, OpenBSD, and NetBSD use the Linux
`sysv_amd64` sources. Their workflow runs `gmake`.

Windows ARM32 (`win_arm`) was removed in 1.0.0. Do not add it back.

The default build uses the checked-in `.S` and `.asm` files.
`STACKMAN_INLINE_ASM` is unset. RISC-V always uses
`switch_riscv64_gcc.S`. When a `.S` is generated from a header, change
the header and the `.S` together. `gen_asm.c` defines
`STACKMAN_INLINE_ASM` as 1. A `.S` `#` line comment is a snapshot of the
header line and goes stale if lines are inserted above the asm. Leave
those comments unless you are regenerating the file.

## Register contract

Volatile registers may change. Do not add a test that treats them as
preserved.

Details and the original audit are in `docs/abi-register-audit.md`.
Limits that are easy to fix the wrong way:

- `arm32` is the soft-float assembly (`switch_arm_gcc.S`, `.arch armv5t`,
  `.fpu softvfp`). It does not spill `d8`–`d15`, and `__ARM_PCS_VFP` is
  a compile error in that header. `armhf` (`switch_armhf_gcc.S`) is the
  hard-float port and is what `__ARM_PCS_VFP` selects. It spills
  `d8`–`d15` and the whole `FPSCR` in `stackman_switch`. Softfp
  (`-mfloat-abi=softfp`) is not a supported platform. It still builds
  as `arm32` and runs, and that assembly does not preserve `FPSCR`.
  Do not add an `FPSCR` spill to `switch_arm_gcc.S` to cover it.
- `aarch64`, `darwin_arm64`, and `win_arm64` do not preserve SME ZA
  across `stackman_switch`. Do not call `__arm_za_disable`. That helper
  commits the outgoing caller's ZA and turns ZA off. Nothing in the
  switch installs the incoming stack's ZA. `stackman_call` returns to
  the same caller and does not need it. Windows ARM64 is a different
  runtime. Do not call the AAPCS64 helper there.
- On Windows ARM64, `x18` is the TEB for the whole time user code runs.
  Spill and restore the saved value. Do not write the live register.
  Do not invent `.seh_*` names for this file.
- A live frame pointer (`x29` on AArch64) is adjusted by the stack delta
  inside the asm. Do not list `x29` as a clobber. Do not adjust ARM32
  `r11` there: `stackman_switch` is built with `omit-frame-pointer`, the
  prologue pushes `r11` and never sets it, and the epilogue reloads the
  saved value. On RISC-V, `s0` is not a frame chain in
  `stackman_switch`, and `stackman_call` uses `s0` for the previous
  stack pointer. Do not adjust it there.
- `stackman_switch` spills `FPCR` and `FPSR` on `aarch64`,
  `darwin_arm64`, and `win_arm64`, and spills `fcsr` on `riscv64`.
  The RISC-V slot is the existing 8-byte pad at the published stack
  pointer. `stackman_call` does not touch these registers.

## Build and test

```bash
make test          # native ABI. Builds lib/$(ABI)/libstackman.a and runs the tests.
make abiname
make PLATFORMFLAGS=-m32 test
make PLATFORM_PREFIX=arm-linux-gnueabi EMULATOR=qemu-arm test
make PLATFORM_PREFIX=arm-linux-gnueabihf EMULATOR=qemu-arm test
```

`make clean` only when switching `PLATFORMFLAGS` or `PLATFORM_PREFIX`.
Success ends with `*** All test suites passed ***`.

Four programs: `bin/test`, `bin/test_cc`, `bin/test_static`,
`bin/test_asm`. `tests/test.c` is `test_01` through `test_08`. `test_04`
and `test_05` are the `stackman_call` contract. `test_07` and `test_08`
pressure the compiler. They pass even when a callee-saved register is
dropped, if the compiler kept the values in memory.

The linker warning `missing .note.GNU-stack section` on
`tests/test_asm_s.o` is expected.

Intel CET: the Makefile adds `-fcf-protection=none` through
`./disable_cet`. Stack switching is incompatible with the shadow stack.
Regenerating asm uses the same flag (`gcc -S -fcf-protection=none`).

Windows, as CI runs it: `msbuild vs2022\stackman.sln /p:Platform=x64`
(also `x86` and `arm64`). Native tests are `vs2022\x64\Debug\test.exe`
and `vs2022\Win32\Debug\test.exe`. The arm64 build is not run. After
`tools/strip-lib.py`, CI rebuilds.

Do not commit rebuilt `lib/` archives, `*.o`, `bin/`, or `tmp/`.
Checked-in libraries under `lib/` are deprecated and go away in a future
major version. `commit-artifacts` in
`.github/workflows/buildcommit.yml` is `if: false`.

## CI

`.github/workflows/buildcommit.yml`, on pull requests to `master`:

- `build-linux-gnu`: AMD64, i386 (`-m32`), arm (`qemu-arm`), armhf
  (`qemu-arm`), aarch64 (`qemu-aarch64`), riscv64 (`qemu-riscv64`).
  Each runs `make test`.
- `build-macos`: `macos-15-intel` (`darwin_x86_64`) and `macos-latest`
  (`darwin_arm64`).
- `build-windows`: VS2022 `x86`, `x64`, and `arm64`. Tests run only
  where `native` is set (`x86` and `x64`).

`.github/workflows/buildbsd.yml` builds and tests FreeBSD, OpenBSD, and
NetBSD with `gmake` on the `sysv_amd64` sources.

A green native `make test` says nothing about another ABI. An assembly
change is checked by the job that builds that file.

## Edits

Keep changes small. Do not add a dependency. Do not bump `stackman.h` or
`CHANGELOG.md` unless asked.

Prefer the checked-in assembly over turning `STACKMAN_INLINE_ASM` on.
`switch_template.h` is the skeleton for a new port.

There is no `main` branch. `master` is the default on both `origin`
(`kristjanvalur/stackman`) and `upstream` (`stackless-dev/stackman`).
