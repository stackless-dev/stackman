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

## Features
- Simple api
  - `stackman_switch()` is the only function.
  - The caller provides a callback and context pointer to customize behaviour.
  - The callback can save the stack and provide the new stack pointer.
  - After the switch, the callback can restore contents of new stack.
  - Application behaviour is entirely defined by the callback.
- Simple implementation
  - The code involving assembly is as simple as possible, allowing for
    straightforward implementation on most platforms.
  - Complex logic and branching is delegated to the C callback.
  - Custom platform code must only do three things:
    1. Save and restore volatile registers on the stack
    2. Adjust stack pointer
    3. Call the callback before and after adjusting the stack pointer.
- Assembly support
  The straightforward and application-agnostic switching allows the switching function to be implemented in full assembler.  This removes
  the risk of inline-assembler doing any sort of unexpected things such
  as in-lining the function or otherwise change the assumptions that the
  function makes about its environment.  This assembly code can be created by the in-line assembler in a controlled environment.
   
## Supported platforms
The current code is distilled out of other work, with the aim of simplifying and
standardizing the api.  The implementation currently works for:
 - Gnu C
   - x86
   - x86-64
   - ARM
   - AARCH64
 - Microsoft Visual Studio:
   - x86
   - x64
   
Other platforms can be easily adapted from both existing implementations for other
projects as well as from example code provided.

## Usage
 - Include `stackman.h` for a decleration of the `stackman_switch()` function
   and the definition of various platform specific macros.  See the documentation
   in the header file for the various macros.
 - Include `stackman_impl.h` from a source file to *define* `stackman_switch()`.
   This can also be an assembler file with the `.S` suffix, see `stackman_impl.h` for
   details.
 - Implement switching semantics via the callback and call `stackman_switch()` from your
   program as appropriate.

## History
This works was originally inspired by *Stackless Python* by [Christian Tismer](https://github.com/ctismer), where the original switching code was
developed.

Later projects, like *gevent* have taken that idea and provided additional platform compatibility but
with a different implementation, making the switching code itself incompatible.

Our work on additional stack-manipulating libraries prompted us to try to distill this functionality in its
rawest form into a separate, low-level, library.  Such that any project, withing to implement *co-routine*-like
behaviour on the C-stack level, could make use of simple, stable code, that can be easily extended for additional
platforms as they come along.
