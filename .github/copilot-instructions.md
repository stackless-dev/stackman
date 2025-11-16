# Copilot Instructions for stackman

## Overview
Low-level C library for stack manipulation (continuations/co-routines). ~600 lines of C + assembly. Zero dependencies. Platforms: Linux (x86/x64/ARM), Windows (x86/x64/ARM). Toolchains: GCC, Clang, MSVC.

## Build Commands (FAST: <1 second)

**Always run in order:**
```bash
make clean      # Clean build artifacts
make all        # Build library (0.1s) → lib/[ABI]/libstackman.a
make test       # Build + run 4 test suites (0.7s)
make abiname    # Print platform ABI (e.g., sysv_amd64)
```

**Success output:** `*** All test suites passed ***`

**Cross-compile x86:** `make PLATFORMFLAGS=-m32 test`
**Cross-compile ARM:** `make PLATFORM_PREFIX=arm-linux-gnueabi- EMULATOR=qemu-arm test`
**Windows:** `msbuild vs2022\stackman.sln /p:Platform=x64` then `vs2022\x64\Debug\test.exe`

## Critical Build Notes

1. **Intel CET:** `-fcf-protection=none` flag REQUIRED (auto-added by disable_cet script). Stack switching incompatible with Shadow Stack.
2. **Libraries ARE Committed:** `lib/**/*.a` and `lib/**/*.lib` are version controlled (unlike typical projects). CI rebuilds and commits them.
3. **Expected Warning:** Linker warning "missing .note.GNU-stack section" in test_asm is NORMAL - ignore it.
4. **Artifacts:** `*.o`, `bin/`, `tmp/` NOT committed. Libraries in `lib/[ABI]/` ARE committed.
5. **Incremental OK:** After code changes, just `make test`. Only clean when switching platforms.

## Project Structure

**Key Directories:**
- `stackman/` - Main source: `stackman.h` (API), `stackman_switch.h`, `stackman_impl.h`, `platforms/` (15+ platform files)
- `tests/` - 4 test files: `test.c` (6 tests), `test_cc.cc`, `test_static.c`, `test_asm.c/.S`
- `lib/[ABI]/` - Pre-built libraries (COMMITTED to git)
- `vs2017/`, `vs2019/`, `vs2022/` - Visual Studio projects
- `tools/` - `abiname.sh`, `strip-lib.py`

**Core API (2 functions only):**
```c
void *stackman_switch(stackman_cb_t callback, void *context);  // Main stack switch
void *stackman_call(stackman_cb_t callback, void *context, void *stack);  // Call with different stack
```

**Architecture:** `platforms/platform.h` detects OS/arch/compiler → includes appropriate `switch_[abi]_[compiler].h/S/asm`

## CI Validation (.github/workflows/buildcommit.yml)

**Triggers:** Push to master/dev, PRs to master

**Jobs:**
1. **build-linux-gnu** (AMD64, i386, arm, aarch64) - installs cross-tools → `make all` → `make test` (qemu for ARM)
2. **build-windows** (x86, x64, arm, arm64) - MSBuild → strip-lib.py → rebuild (MUST rebuild after strip!)
3. **commit-artifacts** (push only) - downloads artifacts → commits libs → pushes

**Local validation:**
```bash
make clean && make test  # Test native platform
git status               # Verify no bin/, tmp/, *.o tracked
```

## Key Patterns & Workarounds

**Making Changes:**
- Platform code: edit `stackman/platforms/switch_*.h` or `.S` - reference `switch_template.h`
- Always run `make test` after changes (fast: 0.7s)
- Test on actual hardware if modifying assembly (arch-specific!)

**Known Issues/Workarounds:**
- **CET:** `-fcf-protection=none` REQUIRED (auto-added by disable_cet script)
- **Inline asm:** May be inlined by optimizer → use separate .S files or volatile pointer (see stackman_impl.h)
- **Stack align:** Use `STACKMAN_SP_ALIGN` macro

**Testing:** 4 test executables, 6 tests each (assertions fail hard). Success: "test_XX ok" + "*** All test suites passed ***"

**Include patterns:**
- User code: `#include "stackman.h"`
- Library impl: `#include "stackman_impl.h"` (once)

## Configuration Files
- **Build:** Makefile (Linux), vs2022/*.vcxproj (Windows)
- **CI:** .github/workflows/buildcommit.yml
- **Linting:** None configured
- **Testing:** `make test` target
- **.gitignore:** Excludes *.o, bin/, tmp/ BUT includes lib/**/*.a, lib/**/*.lib

## Development Tips

1. **Trust these instructions first** - search only if info incomplete/incorrect
2. **Build is FAST** - rebuild freely (clean+test <1s)
3. **Test after every change** - `make test` is fast and comprehensive
4. **Cross-compilation optional** - CI validates all platforms, native x64 sufficient for most changes
5. **Binary files in git** - lib/**/*.a, lib/**/*.lib ARE tracked (expect binary diffs)
6. **Zero dependencies** - don't add any
7. **Minimal changes** - stable library, surgical edits only
8. **Low-level code** - assembly is platform-specific, test on actual hardware
