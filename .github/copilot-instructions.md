# Copilot Instructions for stackman

## Overview
Low-level C library for stack manipulation (continuations/co-routines). ~600 lines of C + assembly. Zero dependencies. **Current version: 1.0.1**

**Supported Platforms (7 ABIs):**
- Linux: sysv_amd64, sysv_i386, arm32 (AAPCS), aarch64 (AAPCS64)
- Windows: win_x86, win_x64, win_arm64

**Note:** Windows ARM32 (win_arm) support was removed in v1.0.0 (Microsoft SDK 10.0.26100.0+ dropped support)

**Toolchains:** GCC, Clang, MSVC (VS2017, VS2019, VS2022)

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
2. **Libraries Deprecated in Repo:** As of v1.0.1, pre-built libraries are NO LONGER automatically committed. Download from [GitHub Releases](https://github.com/stackless-dev/stackman/releases) instead. `lib/` directory will be removed in v2.0.0.
3. **Expected Warning:** Linker warning "missing .note.GNU-stack section" in test_asm is NORMAL - ignore it.
4. **Artifacts:** `*.o`, `bin/`, `tmp/` NOT committed. Libraries available via GitHub Releases.
5. **Incremental OK:** After code changes, just `make test`. Only clean when switching platforms.

## Project Structure

**Key Directories:**
- `stackman/` - Main source: `stackman.h` (API + version macros), `stackman_switch.h`, `stackman_impl.h`, `platforms/` (15+ platform files)
- `tests/` - 4 test files: `test.c` (6 tests), `test_cc.cc`, `test_static.c`, `test_asm.c/.S`
- `lib/[ABI]/` - Pre-built libraries (DEPRECATED - see lib/README.md, will be removed in v2.0.0)
- `vs2017/`, `vs2019/`, `vs2022/` - Visual Studio projects
- `tools/` - `abiname.sh`, `abiname.c`, `strip-lib.py`

**Core API (2 functions only):**
```c
void *stackman_switch(stackman_cb_t callback, void *context);  // Main stack switch
void *stackman_call(stackman_cb_t callback, void *context, void *stack);  // Call with different stack
```

**Architecture:** `platforms/platform.h` detects OS/arch/compiler → includes appropriate `switch_[abi]_[compiler].h/S/asm`

## CI Validation (.github/workflows/buildcommit.yml)

**Triggers:** Push to master/dev, PRs to master, tags (v*.*.*)

**Jobs:**
1. **build-linux-gnu** (sysv_amd64, sysv_i386, arm32, aarch64) - installs cross-tools → `make all` → `make test` (qemu for ARM)
2. **build-windows** (win_x86, win_x64, win_arm64) - MSBuild → strip-lib.py → rebuild (MUST rebuild after strip!)
3. **commit-artifacts** (DISABLED as of v1.0.1) - Previously committed libs to repo, now disabled (if: false)
4. **create-release** (tags only) - Creates GitHub Release with `stackman-{version}.tar.gz` containing all platforms

**Release Archive Contents:**
- `stackman/` - Headers
- `lib/{abi}/` - Pre-built libraries for all 7 platforms
- `tools/` - Platform detection tools
- `Makefile` - For `make abiname`
- Documentation (README.md, LICENSE, CHANGELOG.md)

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
- **Versioning:** stackman/stackman.h - STACKMAN_VERSION_MAJOR/MINOR/PATCH macros + STACKMAN_VERSION string
- **Changelog:** CHANGELOG.md (documents all releases)
- **Linting:** None configured
- **Testing:** `make test` target
- **.gitignore:** Excludes *.o, bin/, tmp/ BUT still includes lib/**/*.a, lib/**/*.lib (deprecated, slated for removal)

## Development Tips

1. **Trust these instructions first** - search only if info incomplete/incorrect
2. **Build is FAST** - rebuild freely (clean+test <1s)
3. **Test after every change** - `make test` is fast and comprehensive
4. **Cross-compilation optional** - CI validates all platforms, native x64 sufficient for most changes
5. **No binary commits** - lib/ directory deprecated, use GitHub Releases
6. **Zero dependencies** - don't add any
7. **Minimal changes** - stable library, surgical edits only
8. **Low-level code** - assembly is platform-specific, test on actual hardware

## Versioning & Releases

**Version Location:** `stackman/stackman.h`
```c
#define STACKMAN_VERSION_MAJOR 1
#define STACKMAN_VERSION_MINOR 0
#define STACKMAN_VERSION_PATCH 1
#define STACKMAN_VERSION "1.0.1"
```

**Release Process:**
1. Update version in `stackman/stackman.h` (MAJOR.MINOR.PATCH)
2. Update `CHANGELOG.md` with changes
3. Commit changes: `git commit -m "Bump version to X.Y.Z"`
4. Create tag: `git tag -a vX.Y.Z -m "Release vX.Y.Z"`
5. Push: `git push upstream master && git push upstream vX.Y.Z`
6. CI automatically creates GitHub Release with pre-built libraries

**Version History:**
- v0.1 (2020-05-18) - Initial release
- v0.2 (retroactive tag) - Pre-1.0 development state
- v1.0.0 (2025-11-16) - First stable release, removed Windows ARM32
- v1.0.1 (2025-11-16) - Disabled library commits, use GitHub Releases
- v2.0.0 (future) - Will remove lib/ directory entirely
