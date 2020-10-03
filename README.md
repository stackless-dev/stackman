# stackman
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

- [gevent](https://github.com/gevent/gevent)
- [Stackless Python](https://github.com/stackless-dev/stackless/)

However, each of these perform things slightly differently and individual platform
implementations have to be maintained for each of them.

This library aims to provide a simple api that can be shared among all projects
that manipulate the C stack.

Additionally, it provides a set of pre-assembled libraries for the most common
platforms so that no assembly steps are required by users.

## Features
- Simple api
  - `stackman_switch()` is the main function.
  - The caller provides a callback and context pointer to customize behaviour.
  - The callback can save the stack and provide the new stack pointer.
  - After the switch, the callback can restore contents of new stack.
  - Application behaviour is entirely defined by the callback.
- Simple implementation
  - The code involving assembly is as simple as possible, allowing for
    straightforward implementation on most platforms.
  - Complex logic and branching is delegated to the C callback.
  - Custom platform code must only do three things:
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
 - microsoft_cdecl (32 bits)
 - microsoft_x64
 - sysv_i386 (linux)
 - sysv_amd64 (linux)
 - arm32 (32 bit arm)
 - aarch64 (64 bit arm)

Supported toolchains:
 - Gnu C
 - clang
 - Microsoft Visual Studio
   
Other platforms can be easily adapted from both existing implementations for other
projects as well as from example code provided.

### Intel CET
Intel's Conontrol-Flow Enforcement Technology is incompatible with stack switching
because it imploys a secondary Shadow Stack, that the user-mode program cannot
modify.  Unexpected return flow after a stack switch would cause the processor
to fault.  Because of this, we need to mark any assembly code as not CET compatible.  Modern compilers are beginning to generate CET compatible objects and
once supporting CPUs start to arrive, processes which consist entirely of CET compatible code may be run in such a protected environment.  See https://software.intel.com/content/www/us/en/develop/articles/technical-look-control-flow-enforcement-technology.html for more information
 
## Usage
 - Include `stackman.h` for a decleration of the `stackman_switch()` function
   and the definition of various platform specific macros.  See the documentation
   in the header file for the various macros.
 - Include `stackman_impl.h` from a source file to *define* `stackman_switch()`.
   This can also be an assembler file with the `.S` suffix, see `stackman_impl.h` for
   details.
 - Alternatively, link with libstackman.a or libstackman.lib for your platform.
 - Implement switching semantics via the callback and call `stackman_switch()` from your
   program as appropriate.  See tests/test.c for examples.

## History
This works was originally inspired by *Stackless Python* by [Christian Tismer](https://github.com/ctismer), where the original switching code was
developed.

Later projects, like *gevent* have taken that idea and provided additional platform compatibility but
with a different implementation, making the switching code itself incompatible.

Our work on additional stack-manipulating libraries prompted us to try to distill this functionality in its
rawest form into a separate, low-level, library.  Such that any project, withing to implement *co-routine*-like
behaviour on the C-stack level, could make use of simple, stable code, that can be easily extended for additional
platforms as they come along.
