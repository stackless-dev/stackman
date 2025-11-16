# Pre-built Libraries

## ⚠️ DEPRECATED - Slated for Removal

The pre-built library files in this directory are **deprecated** and will be removed in a future release.

### Why are they being removed?

1. **Binary files in git cause issues:**
   - Merge conflicts that are difficult to resolve
   - Repository bloat (git doesn't compress binaries well)
   - Noise commits from automated builds

2. **Better alternatives are available:**
   - Download libraries from [GitHub Releases](https://github.com/stackless-dev/stackman/releases)
   - Build from source (fast: <1 second with `make all`)
   - Use workflow artifacts for development builds

### Migration Guide

**For stable versions:**
- Download pre-built libraries from [Releases](https://github.com/stackless-dev/stackman/releases)
- Each release includes all platform libraries + combined archive

**For development:**
- Build locally: `make all` (requires appropriate toolchain)
- Download workflow artifacts from GitHub Actions runs
- Libraries are available as build artifacts for 90 days

**Example:**
```bash
# Download release
wget https://github.com/stackless-dev/stackman/releases/download/v1.0.0/libstackman.a

# Or build from source (fast!)
make clean && make all
```

### Timeline

- **v1.0.0**: Libraries still committed (this directory)
- **v1.0.1**: Library commits disabled, this directory marked deprecated
- **v2.0.0**: This directory will be removed entirely

### Current Platforms

The following platforms are available in releases:

- **Linux**: sysv_amd64, sysv_i386, arm32 (AAPCS), aarch64 (AAPCS64)
- **Windows**: win_x86, win_x64, win_arm64

See the [main README](../README.md) for more information.
