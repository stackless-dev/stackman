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
 * STACKMAN_ABI - the ABI being used.
 * STACKMAN_PLATFORM_ASM - If pure assembler, the filename of the assembly code.
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
 * Also provide defaults for stack "fullness" and alignment.  The most
 * common stacks are "full" "descending"
 */
#ifndef STACKMAN_STACK_DIR
#define STACKMAN_STACK_DIR 0 	/* stack pointer grows "downwards" in memory. */
#endif
#ifndef STACKMAN_STACK_FULL
#define STACKMAN_STACK_FULL 1	/* stack pointer points to a "full" position */
#endif
#ifndef STACKMAN_STACK_ALIGN
#define STACKMAN_STACK_ALIGN 4  /* stack pointer needs to be aligned to this */
#endif

/* define common macros for stack type.  Currently only platforms with FULL_DESCENDING are supported. */
#define STACKMAN_STACK_FULL_DESCENDING 0
#define STACKMAN_STACK_EMPTY_DESCENDING 0
#define STACKMAN_STACK_FULL_ASCENDING 0
#define STACKMAN_STACK_EMPTY_ASCENDING 0
#if STACKMAN_STACK_DIR == 0
#if STACKMAN_STACK_FULL == 1
#undef  STACKMAN_STACK_FULL_DESCENDING
#define STACKMAN_STACK_FULL_DESCENDING 1
#else
#undef  STACKMAN_STACK_EMPTY_DESCENDING
#define STACKMAN_STACK_EMPTY_DESCENDING 1
#endif
#else
#if STACKMAN_STACK_FULL == 0
#undef  STACKMAN_STACK_FULL_ASCENDING
#define STACKMAN_STACK_FULL_ASCENDING 1
#else
#undef  STACKMAN_STACK_EMPTY_ASCENDING
#define STACKMAN_STACK_EMPTY_ASCENDING 1
#endif
#endif


/* align a stack pointer to the righ alighment, either nudging it up or down */
#define STACKMAN_SP_ALIGN_DOWN(a) (((intptr_t)(a) & ~(STACKMAN_STACK_ALIGN-1)))
#define STACKMAN_SP_ALIGN_UP(a)	  (((intptr_t)((a)+STACKMAN_STACK_ALIGN-1) & ~(STACKMAN_STACK_ALIGN-1)))

#if STACKMAN_STACK_DIR == 0
#define STACKMAN_SP_FURTHEST      ((void*) ^(intptr_t)-1)
#define STACKMAN_SP_NEAREST       ((void*) 0)
#define STACKMAN_SP_LS(a, b)      ((a) < (b))     /* to compare stack position */
#define STACKMAN_SP_LE(a, b)      ((a) <= (b))    /* to compare stack position */
#define STACKMAN_SP_ADD(a, b)     ((a) + (b))     /* to add offset to stack pointer */
#define STACKMAN_SP_DIFF(a, b)    ((a) - (b))     /* to subtract stack pointers */
#define STACKMAN_SP_ALIGN(a)      STACKMAN_SP_ALIGN_DOWN(a)
#else
/* upwards growing stacks */
#define STACKMAN_SP_FURTHEST      ((void*) 0)
#define STACKMAN_SP_NEAREST       ((void*) ^(intptr_t)-1)
#define STACKMAN_SP_LS(a, b)      ((a) > (b))     /* to compare stack position */
#define STACKMAN_SP_LE(a, b)      ((a) >= (b))    /* to compare stack position */
#define STACKMAN_SP_ADD(a, b)     ((a) - (b))     /* to add offset to stack pointer */
#define STACKMAN_SP_DIFF(a, b)    ((b) - (a))     /* to subtract stack pointers */
#define STACKMAN_SP_ALIGN(a)      STACKMAN_SP_ALIGN_UP(a)
#endif

#endif /* STACKMAN_H */
