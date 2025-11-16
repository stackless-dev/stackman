# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
  - ARM32 Windows devices (Windows RT) are obsolete
  - ARM64 Windows remains fully supported

### Fixed
- Fixed typos in documentation and source files
- Corrected "callee-stored" → "callee-saved" terminology

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

[1.0.0]: https://github.com/stackless-dev/stackman/releases/tag/v1.0.0
[0.1]: https://github.com/stackless-dev/stackman/releases/tag/v0.1
