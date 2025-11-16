# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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

[1.2.0]: https://github.com/stackless-dev/stackman/releases/tag/v1.2.0
[1.1.0]: https://github.com/stackless-dev/stackman/releases/tag/v1.1.0
[1.0.1]: https://github.com/stackless-dev/stackman/releases/tag/v1.0.1
[1.0.0]: https://github.com/stackless-dev/stackman/releases/tag/v1.0.0
[0.2]: https://github.com/stackless-dev/stackman/releases/tag/v0.2
[0.1]: https://github.com/stackless-dev/stackman/releases/tag/v0.1
