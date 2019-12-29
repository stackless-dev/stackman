# stackman
Simple low-level stack manipulation API and implementation for common platforms

## Purpose
This library aims to provide a basic API to perfom stack manipulation
on various platforms.  Stack manipulation involves changing the stack
pointer while optionally saving and restoring the stack contents.

Manipulating the stack pointer allows the implementation of _continuations_
and _long jump_ functionality in C.  However, this invariably involves
low-level assembly code, to save and restore machine registers and manipulate
the stack pointer itself.  Each machine platform, and tool suite, will need
appropriate customization for it to work.

Various projects perform stack manipulation to provide _co-routine_ functionality:

- [gevent](https://github.com/gevent/gevent)
- [Stackless Python](https://github.com/stackless-dev/stackless/)

However, each of those perform things slightly differently and work has to be
maintained for each of them.

This library aims to provide a simple api that can be shared among all projects
that manipulat the C stack.

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

## History
This works is originally inspired by *Stackless Python* by @ctismer, where the original switching code was
developed.

Later projects, like *gevent* have taken that idea and provided additional platform compatibility but
with different implementation, making the switching code incompatible.

Our work on additional stack-manipulating libraries prompted us to try to distill this functionality in its
rawest form into a separate, low-level, library.  Such that any project, withing to implement *co-routine*-like
behaviour on the C-stack level, could make use of simple, stable code, that can be easily extended for additional
platforms as they come along.


