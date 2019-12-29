#pragma once
#ifndef __STACKMAN_H__
#define __STACKMAN_H__

#include <stdint.h>
#include "platforms/platform.h"

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
#define STACKMAN_SP_SUB(a, b)     ((a) - (b))     /* to subtract stack pointers */
#else
#define STACKMAN_SP_FURTHEST      ((void*) 0)
#define STACKMAN_SP_NEAREST       ((void*) ^(intptr_t)-1)
#define STACKMAN_SP_LE(a, b)      ((b) <= (a))    /* to compare stack position */
#define STACKMAN_SP_SUB(a, b)     ((b) - (a))     /* to subtract stack pointers */
#endif

/* raw stack switching function. The caller supplies a callback
 * and context to perform determine the new stack pointer and
 * perform cleanup of old stack and initialization of the new
 * stack.
 * The platform specific implementation of the function is responsible
 * for saving and restoring any machine registers that are assumed
 * to be unchanged across function calls and to change the actual
 * stack pointer.
 *
 * The implementation must simply:
 * 1) store all cpu state on the stack (callee-stored registers, etc)
 * 2) call the callback with the context, current stack pointer and 
 *    phase==0.  This allows the application to do additional
 *    cleanup and determine the new stack pointer.
 * 3) replace the stack pointer with the returned value from the callback.
 * 4) call the callback again with new stack pointer and phase==1.  This
 *    allows the application to initialize its state after the switch.
 * 5) Pop cpu state back from stack.
 * 6) return the result from the second call of the callback.
 *
 * Note that the callback and context are _not_ guaranteed
 * to be stack local.  That is, even if the stack pointer is modified
 * according to the first call to the callback, the same callback and
 * context will be used during the second callback, in the new stack.
 * Thus, both stacks must really belong to the same program.  This
 * restriction could be lifted by pushing both values onto the stack
 * before switching but the added complexity of platform implementations
 * doesn't seem to warrant that for such an exotic use case.
 *
 * an appropriate implementation is included b
 * platf_tealet/tealet_platformselect.h
 * A template implementation is available in
 * platf_tealet/switch_template.h
 */

/* function signature to save/restore state in a stack.
 * 'context' contains information
 * for the function, 'stack_pointer' is the lower level
 * of the stack.  Returns new stack pointer.
 */
typedef void *(*stackman_cb_t)(
	void *context, void *stack_pointer, int stage);
/* The actual stack switching function.
 * It saves state, switches stack pointer, and restores state
 */
void *stackman_switch(stackman_cb_t callback, void *context);

#endif /* __STACKMAN_H__ */