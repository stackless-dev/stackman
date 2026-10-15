# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.3.0] - 2026-10-15

### Added
- `armhf`, a 32-bit ARM hard-float ABI (AAPCS-VFP). `__ARM_PCS_VFP` selects it. Soft-float stays `arm32`. CI and the release archive include `lib/armhf`.
- `docs/platform-abi.md`, the current contract for switching and register spills. `docs/abi-register-audit.md` remains the historical record of that audit.
- `test_09` checks that the caller's rounding mode survives `stackman_switch`, including a switch onto a saved stack. Soft-float `arm32` does not spill `FPSCR`, and the test expects the callback's mode to remain.
- FreeBSD, OpenBSD and NetBSD (amd64, `sysv_amd64` ABI) are built and tested on every commit by a `build and test bsd` workflow.

### Changed
- `stackman_switch` spills floating-point control and status state and reloads it after the restore callback. `stackman_call` does not touch those registers.
- `armhf` spills the whole `FPSCR`.
- `aarch64` and `darwin_arm64` spill `FPCR` and `FPSR`.
- `win_arm64` spills `FPSR` as well as `FPCR`.
- `riscv64` spills the whole `fcsr`.
- `sysv_i386` spills the x87 control word. `win_x64` spills `MXCSR`.
- `win_x86` spills the x87 control word and `MXCSR`, so older compilers that emit x87 and current MSVC, which emits SSE2, each keep their rounding mode.
- Softfp (`-mfloat-abi=softfp`) is not a supported platform. That build selects `arm32` and does not preserve `FPSCR`.
- CI test steps stop after one minute.

### Fixed
- RISC-V `stackman_call` leaves a null stack unchanged and passes the previous stack pointer to the callback.
- Windows ARM64 no longer uses the live TEB register `x18` as a scratch in `stackman_switch` or `stackman_call`.
- AArch64 `stackman_switch` keeps the frame pointer attached across a stack switch, and the unwind info covers the `FPCR` slot.
- ABI detection now works on the BSDs: `tools/abiname.sh` used a `mktemp` template with three `X`s, which BSD `mktemp(1)` rejects.

### Removed
- Leftover Windows ARM32 sources, Visual Studio configurations, and `lib/win_arm`. Version 1.0.0 had already dropped that platform.

## [1.2.5] - 2026-06-23

### Changed
- macOS release libraries now use per-architecture deployment targets: `10.13` for Intel and `11.0` for Apple Silicon.

## [1.2.4] - 2026-06-23

### Fixed
- macOS release libraries are built with `MACOSX_DEPLOYMENT_TARGET=14.0` so they are not tagged with the runner's current macOS version.

## [1.2.3] - 2026-06-06

### Added
- Added explicit `STACKMAN_ABI` override support in the Makefile and ABI probe script for deterministic cross-build packaging

### Changed
- Normalized Windows ARM64 ABI naming to `win_arm64` and kept compatibility mapping for legacy `win_aarch64` inputs
- ABI probe diagnostics now query compiler target triple and emit clearer mismatch guidance when target intent is not fully propagated through wrapper/toolchain settings
- Target triple probing now prefers `CC` with `CFLAGS` (with fallback probing) so clang-style cross-target flags are reflected in secondary detection

## [1.2.2] - 2026-06-05

### Changed
- Windows MSVC library projects now compile with `/Zl` to omit embedded default runtime library directives from generated object files
- Windows static libraries no longer carry `/DEFAULTLIB:"MSVCRTD"` metadata, reducing unintended CRT coupling for downstream linkers

## [1.2.1] - 2026-05-09

### Changed
- CI: pinned Windows runner to `windows-2022` (from `windows-latest`) to keep VS2022 v143 toolset availability stable for ARM64 builds

### Fixed
- Fixed amd64 (sysv_amd64 GCC) restore callback stack-pointer argument to pass the active switched stack pointer
- Added assertion coverage in `tests/test.c` to detect incorrect restore callback stack pointer on amd64

## [1.2.0] - 2025-11-16

### Added
- RISC-V platform support
  - `riscv64` - RISC-V 64-bit (LP64D ABI with hardware double-precision floating point)
- Platform detection for RISC-V in `platforms/platform.h` using `__riscv` and `__riscv_xlen` macros
- Assembly implementation in `platforms/switch_riscv64_gcc.S`
  - Full register preservation (s0-s11, fs0-fs11, ra)
  - 16-byte stack alignment per RISC-V ABI
  - Both `stackman_switch` and `stackman_call` functions
- RISC-V build job in CI workflow with qemu-riscv64 emulation
- RISC-V library included in release archives

### Changed
- Release archives now contain 10 platform libraries (was 9)

## [1.1.0] - 2025-11-16

### Added
- macOS platform support
  - `darwin_x86_64` - macOS on Intel (x86_64)
  - `darwin_arm64` - macOS on Apple Silicon (ARM64)
- Platform detection for macOS in `platforms/platform.h` using `__APPLE__` and `__aarch64__` macros
- Mach-O assembly compatibility for both x86_64 and ARM64
  - Conditional assembly macros for ELF vs Mach-O object formats
  - Disabled CFI directives on macOS (different semantics than Linux)
  - Symbol name mangling with leading underscore for Mach-O
- macOS build jobs in CI workflow (macos-15-intel and macos-latest runners)
- macOS libraries included in release archives

### Changed
- Assembly files (`switch_x86_64_gcc.S`, `switch_aarch64_gcc.S`) now support both Linux (ELF) and macOS (Mach-O)
- `Makefile` detects Darwin and disables `-static` flag (not supported on macOS)
- `tools/abiname.sh` improved to handle stale temp files on macOS
- Release archives now contain 9 platform libraries (was 7)

### Fixed
- CFI macro definitions in `switch_aarch64_gcc.S` for Linux builds

## [1.0.1] - 2025-11-16

### Changed
- Disabled automatic library commits to repository
- Pre-built libraries now available exclusively via [GitHub Releases](https://github.com/stackless-dev/stackman/releases)
- Added `lib/README.md` documenting deprecation timeline

### Deprecated
- `lib/` directory in repository - will be removed in v2.0.0
- Committing binary library files to git (causes bloat and merge conflicts)

## [1.0.0] - 2025-11-16

### Added
- Version macros in `stackman.h`: `STACKMAN_VERSION_MAJOR`, `STACKMAN_VERSION_MINOR`, `STACKMAN_VERSION_PATCH`, `STACKMAN_VERSION`, `STACKMAN_VERSION_NUMBER`
- Automated release workflow for tagged versions

### Changed
- Updated GitHub Actions to v4 (from v2)
- README updated with complete platform list, CI information, and release documentation

### Removed
- **BREAKING**: Dropped Windows ARM32 (win_arm) support
  - Microsoft Windows SDK 10.0.26100.0+ no longer supports 32-bit ARM development
  - Last SDK version supporting ARM32 was Windows SDK 10.0.22621 (Windows 11 SDK, version 22H2)
  - ARM32 Windows devices (Windows RT) are obsolete
  - ARM64 Windows remains fully supported

### Fixed
- Fixed typos in documentation and source files
- Corrected "callee-stored" → "callee-saved" terminology

## [0.2] - 2024-2025

### Changed
- Updated to VS2022 project files
- Updated GitHub Actions dependencies
- Automated library builds committed to repository

## [0.1] - 2020-05-18

### Added
- Core stack manipulation API: `stackman_switch()` and `stackman_call()`
- Support for 8 platforms:
  - Linux: sysv_amd64, sysv_i386, arm32 (AAPCS), aarch64 (AAPCS64)
  - Windows: win_x86, win_x64, win_arm (32-bit ARM), win_arm64
- Compiler support: GCC, Clang, MSVC (VS2017, VS2019, VS2022)
- Pre-built libraries for all supported platforms
- Inline assembly and separate assembly file options
- Cross-compilation support for Linux (x86, ARM32, ARM64)
- QEMU-based testing for ARM platforms in CI
- Comprehensive test suite (test.c, test_cc.cc, test_static.c, test_asm.c)
- GitHub Actions CI for automated building and testing
- Visual Studio project files (VS2017, VS2019, VS2022)

[Unreleased]: https://github.com/stackless-dev/stackman/compare/v1.3.0...HEAD
[1.3.0]: https://github.com/stackless-dev/stackman/releases/tag/v1.3.0
[1.2.5]: https://github.com/stackless-dev/stackman/releases/tag/v1.2.5
[1.2.4]: https://github.com/stackless-dev/stackman/releases/tag/v1.2.4
[1.2.3]: https://github.com/stackless-dev/stackman/releases/tag/v1.2.3
[1.2.2]: https://github.com/stackless-dev/stackman/releases/tag/v1.2.2
[1.2.1]: https://github.com/stackless-dev/stackman/releases/tag/v1.2.1
[1.2.0]: https://github.com/stackless-dev/stackman/releases/tag/v1.2.0
[1.1.0]: https://github.com/stackless-dev/stackman/releases/tag/v1.1.0
[1.0.1]: https://github.com/stackless-dev/stackman/releases/tag/v1.0.1
[1.0.0]: https://github.com/stackless-dev/stackman/releases/tag/v1.0.0
[0.2]: https://github.com/stackless-dev/stackman/releases/tag/v0.2
[0.1]: https://github.com/stackless-dev/stackman/releases/tag/v0.1
