#pragma once
#ifndef STACKMAN_H
#define STACKMAN_H

/* the main include file.  The following macros can be defined before including
 * STACKMAN_OPTIONAL - Do not error if the platform isn't supported
 * STACKMAN_VERBOSE  - Emit the found platform to output
 * STACKMAN_PREFER_ASM - define as 1 to prefer assembly code to inline-assembly
 *
 * After include, the following may be defined
 * STACKMAN_PLATFORM - If defined, the platform in use
 *    STACKMAN_PLATFORM_ASM - If pure assembler, the filename of the assembly code.
 *
 * For the file providing the implementation of stackman_switch(), include
 * "stackman_impl.h" instead.
 */ 

#include "platforms/platform.h"
#include "stackman_switch.h"

/* The default stack direction is downwards, 0, but platforms
 * can redefine it to upwards growing, 1.
 * Since we support both architectures with descending and
 * ascending stacks, we use the terms "near" and "far"
 * to describe stack boundaries.  In a typical architecture
 * with descending stacks, "near" corresponds to a low
 * address and "far" to a high address.
 */
#ifndef STACKMAN_DIRECTION
#define STACKMAN_DIRECTION 0 
#endif  

#if STACK_DIRECTION == 0
#define STACKMAN_SP_FURTHEST      ((void*) ^(intptr_t)-1)
#define STACKMAN_SP_NEAREST       ((void*) 0)
#define STACKMAN_SP_LE(a, b)      ((a) <= (b))    /* to compare stack position */
#define STACKMAN_SP_ADD(a, b)     ((a) + (b))     /* to add offset to stack pointer */
#define STACKMAN_SP_DIFF(a, b)    ((a) - (b))     /* to subtract stack pointers */
#else
#define STACKMAN_SP_FURTHEST      ((void*) 0)
#define STACKMAN_SP_NEAREST       ((void*) ^(intptr_t)-1)
#define STACKMAN_SP_LE(a, b)      ((a) >= (b))    /* to compare stack position */
#define STACKMAN_SP_ADD(a, b)     ((a) - (b))     /* to add offset to stack pointer */
#define STACKMAN_SP_DIFF(a, b)    ((b) - (a))     /* to subtract stack pointers */
#endif

#endif /* STACKMAN_H */
