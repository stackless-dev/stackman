[![build test and commit](https://github.com/kristjanvalur/stackman/actions/workflows/buildcommit.yml/badge.svg)](https://github.com/kristjanvalur/stackman/actions/workflows/buildcommit.yml)

# stackman

**Version 1.2.0**

Simple low-level stack manipulation API and implementation for common platforms

## Purpose

This library aims to provide a basic API to perfom stack manipulation
on various platforms.  Stack manipulation involves changing the machine stack
pointer while optionally saving and restoring the stack contents.

Manipulating the stack pointer allows the implementation of _continuations_
and _long jump_ functionality in C.  However, this invariably involves
low-level assembly code, to save and restore machine registers and manipulate
the stack pointer itself.  Each machine platform, and tool suite, will need
appropriate customization for it to work.

Various projects perform stack manipulation to provide _co-routine_ functionality:

- [greenlet](https://github.com/python-greenlet/greenlet)
- [Stackless Python](https://github.com/stackless-dev/stackless/)

However, each of these perform things slightly differently and individual platform
implementations have to be maintained for each of them.

This library aims to provide a simple api that can be shared among all projects
that manipulate the C stack.

Additionally, it provides a set of pre-assembled libraries for the most common
platforms so that no assembly steps are required by users.

## Features

- Simple api

  - `stackman_switch()` and `stackman_call()` are the only functions.
  - The caller provides a callback and context pointer to customize behaviour.

- Simple implementation

  - The code involving assembly is as simple as possible, allowing for
    straightforward implementation on most platforms.
  - Complex logic and branching is delegated to the C callback.
  - Custom platform assembly code must only do three things:
    1. Save and restore volatile registers and stack state on the stack
    2. Call the callback twice with the current stack pointer
    3. Set the stack pointer to the value returned by the first callback.

- Assembly support

  The straightforward and application-agnostic switching allows the switching function to be implemented in full assembler.  This removes
  the risk of inline-assembler doing any sort of unexpected things such
  as in-lining the function or otherwise change the assumptions that the
  function makes about its environment.  This assembly code can be created by the in-line assembler in a controlled environment.

- No dependencies  

  The library merely provides stack switching.  It consist only of a couple of functions with no dependencies.

- Stable  

  There is no need to add or modify functionality.

- Libraries provided.  

  The aim is to provide pre-assembled libraries for the most popular platforms. This relieves other tools that want to do stack
  manipulation from doing any sort of assembly or complex linkage.  Just include the headers and link to the appropriate library.

## Supported platforms

The current code is distilled out of other work, with the aim of simplifying and
standardizing the api.  A number of ABI specifications is supported, meaning architecture and
calling convention, plus archive format:

 - **Linux (System V ABI)**
   - sysv_i386 (32-bit x86)
   - sysv_amd64 (64-bit x86_64)
   - arm32 (32-bit ARM, AAPCS)
   - aarch64 (64-bit ARM, AAPCS64)
   - riscv64 (64-bit RISC-V, LP64D ABI)
 - **macOS (Darwin)**
   - darwin_x86_64 (Intel)
   - darwin_arm64 (Apple Silicon)
 - **Windows**
   - win_x86 (32-bit)
   - win_x64 (64-bit)
   - win_arm64 (64-bit ARM)

All platforms are automatically built and tested by GitHub Actions CI on every commit.

### Supported toolchains:

 - Gnu C
 - clang
 - Microsoft Visual Studio (VS2017, VS2019, VS2022)
   
Other platforms can be easily adapted from both existing implementations for other
projects as well as from example code provided.

## API

There are two functions that make up the stackman library: `stakman_switch()` and `stackman_call()` who
both take a `stackman_cb_t` callback:

```C
typedef void *(*stackman_cb_t)(
	void *context, int opcode, void *stack_pointer);
void *stackman_switch(stackman_cb_t callback, void *context);
void *stackman_call(stackman_cb_t callback, void *context, void *stack);
```

### stackman_switch()

This is the main _stack manipulation_ API.  When called, it will call `callback` function twice:

1. First it calls it with the current opcode `STACKMAN_OP_SAVE`, passing the current `stack_pointer` to
the callback.  This gives the callback the opportunity to _save_ the stack data somewhere.  The callback
can then return a **different** stack pointer.
2. It takes the returned value from the calback and replaces the CPU _stack pointer_ with it.
3. It calls the callback a second time, with the opcode `STACKMAN_OP_RESTORE` and the new stack pointer.
This gives the callback the opportunity to replace the data on the stack with previously saved data.
4. It returns the return value from the second call to the callback function.

The `context` pointer is passed as-is to the callback, allowing it access to user-defined data.

Depending on how the callback function is implemented, this API can be used for a number of things, like
saving a copy of the stack, perform a stack switch, query the stack pointer, and so on.

### stackman_call()

This is a helper function to call a callback function, optionally providing it with a different stack to
use.

1. It saves the current CPU stack pointer.  If `stack` is non-zero, it will replace the stackpointer
with that value.
2. It calls the callback function with the opcode `STACKMAN_OP_CALL`.
3. It replaces the stack pointer with the previously saved value and returns the return value from the callback.

This function is useful for at least three things:

- To move the call chain into a custom stack area, some heap-allocated block, for example.
- To query the current stack pointer
- To enforce an actual function call with stack pointer information.

The last feature is useful to bypass any in-lining that a compiler may do, when one really wants
a proper function call with stack, for example, when setting up a new stack entry point.

## Usage

 - Include `stackman.h` for a decleration of the `stackman_switch()` function
   and the definition of various platform specific macros.  See the documentation
   in the header file for the various macros.
 - Implement switching semantics via the callback and call `stackman_switch()` from your
   program as appropriate.  See tests/test.c for examples.

There are two basic ways to add the library to your project: Using a static library or inlining the code.

### static library (preferred)

 - Download pre-built libraries from the [Releases page](https://github.com/kristjanvalur/stackman/releases) for your platform
 - Alternatively, link with the `libstackman.a` or `stackman.lib` libraries in the `lib/` directory if you've cloned the repository
 - Pre-built libraries are available for all supported platforms (9 ABIs total: 4 Linux, 2 macOS, 3 Windows)
 - Libraries are automatically rebuilt by CI and committed to the repository for easy integration

### inlined code

 - Include `stackman_impl.h` in one of your .c source files to provide inline assembly.
 - Include `stackman_impl.h` in an assembly (.S) file in your project to include assembly code.
 - (Windows) Include `stackman_s.asm` in an assembly (.asm) file in your project.

In the case of inlined code, it can be specified to prefer in-line assembly and static linkage
over separate assembly language source.

## Continuous Integration

The project uses GitHub Actions to automatically:
- Build libraries for all 9 supported platforms (Linux: AMD64, i386, ARM32, ARM64; macOS: x86_64, ARM64; Windows: x86, x64, ARM64)
- Run test suites on all platforms (using QEMU emulation for ARM on Linux)
- Commit updated libraries back to the repository on successful builds (for development branches)
- Create GitHub Releases with downloadable libraries when version tags are pushed

### Releases

Tagged versions (e.g., `v1.0.0`) automatically trigger:
- Build of all platforms
- Creation of a GitHub Release
- Upload of individual library files and a combined archive containing all platforms + headers

Download stable releases from: https://github.com/kristjanvalur/stackman/releases

See `.github/workflows/buildcommit.yml` for the complete CI configuration.

## Development

### Adding new platforms

1. Modify `platform.h` to identify the platform environment.  Define an ABI name and
   include custom header files.
2. Use the `switch_template.h` to help build a `switch_ABI.h` file for your ABI.
3. Provide an assembler version, `switch_ABI.S` by compiling the `gen_asm.c` file for your platform.
4. Provide cross-compilation tools for linux if possible, by modifying the `Makefile`

### Cross-compilation

Linux on x86-64 can be used to cross compile for x86 and ARM targets.  This is most useful to generate assembly code, e.g. when compiling
stackman/platform/gen_asm.c

 - x86 requires the -m32 flag to compilers and linkers.
 - arm32 requires to use the arm-linux-gnueabi-* tools, including cc and linker
 - aarch64 requires the aarch64-linux-gnu-* tools.

The x86 tools require the **gcc-multilib** and **g++-multilib** packages to be installed.  They, however, can't co-exist with the **gcc-arm-linux-gnueabi** or
**gcc-aarch64-linux-gnu** packages on some distributions, and so development for these
platforms may need to be done independently.

#### Cross compiling for x86 (32 bit) on Linux

 - install __gcc-multilib__ and __g++-multilib__
 - *compile* **gen_asm.c** using `gcc -m32`
 - *make* using  `make PLATFORMFLAGS=-m32  test`

#### Cross compiling for ARM (32 bit) on Linux

 - install __gcc-arm-linux-gnueabi__ and __g++-arm-linux-gnueabi__
 - install __qemu-user__ for hardware emulation 
 - *compile* **gen_asm.c** using `arm-linux-gnueabi-gcc`
 - *make* using  `make PLATFORM_PREFIX=arm-linux-gnueabi- EMULATOR=qemu-arm test`

#### Cross compiling for Arm64 on Linux

 - install **gcc-aarch64-linux-gnu** and **g++-aarch64-linux-gnu**
 - install __qemu-user__ for hardware emulation 
 - *compile* using `aarch64-linux-gnu-gcc`
 - *make* using `make PLATFORM_PREFIX=aarch64-linux-gnu- EMULATOR=qemu-aarch64 test`

## A note about Intel CET

Intel's *Control-Flow Enforcement Technology* is incompatible with stack switching
because it employs a secondary *Shadow Stack*, that the user-mode program cannot
modify.  Unexpected return flow after a stack switch would cause the processor
to fault.  Because of this, we need to mark any assembly code as **not CET compatible** by
adding special compiler flags to supporting compilers (currently modern GNU C).
Modern compilers are beginning to generate CET compatible objects and
once supporting CPUs start to arrive, processes which consist entirely of CET compatible
code may be run in such a protected environment.
See https://software.intel.com/content/www/us/en/develop/articles/technical-look-control-flow-enforcement-technology.html for more information
 
## History

This works was originally inspired by *Stackless Python* by [Christian Tismer](https://github.com/ctismer), where the original switching code was
developed.

Later projects, like *gevent/greenlet* have taken that idea and provided additional platform compatibility but
with a different implementation, making the switching code itself incompatible.

Our work on additional stack-manipulating libraries prompted us to try to distill this functionality in its
rawest form into a separate, low-level, library.  Such that any project, wishing to implement *co-routine*-like
behaviour on the C-stack level, could make use of simple, stable code, that can be easily extended for additional
platforms as they come along.
