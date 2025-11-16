#pragma once
#ifndef STACKMAN_SWITCH_H
#define STACKMAN_SWITCH_H

/* Must include the platform here to know if the switch function
 * is implemented in assembler or nt
 */
#include "platforms/platform.h"

/* we can request that C functions be implemented with static linkage
 * when including in other libraries
 */
#if defined(STACKMAN_LINKAGE_STATIC)
#if !defined(STACKMAN_ASSEMBLY_SRC) && !STACKMAN_SWITCH_C
#define STACKMAN_LINKAGE_SWITCH static
#endif
#endif

#if !defined(STACKMAN_LINKAGE_SWITCH)
/* then it is extern */
#if defined(__cplusplus)
#define STACKMAN_LINKAGE_SWITCH extern "C"
#else
#define STACKMAN_LINKAGE_SWITCH extern
#endif
#endif

/* do we need to have indirection for stackman_switch?
 * we do, if the implementation is inline and we are not
 * building a library, because an optimizing compiler may
 * decide to inline functions that contain in-line assembler
 */
#if (!defined(STACKMAN_ASSEMBLY_SRC) && !defined(STACKMAN_BUILD_LIB))
#define STACKMAN_SWITCH_NEED_INDIRECT 1
#else
#define STACKMAN_SWITCH_NEED_INDIRECT 0
#endif

#if STACKMAN_SWITCH_NEED_INDIRECT
#define STACKMAN_SWITCH_INASM_NAME _stackman_switch_inasm
#define STACKMAN_LINKAGE_SWITCH_INASM static
#else
#define STACKMAN_SWITCH_INASM_NAME stackman_switch
#define STACKMAN_LINKAGE_SWITCH_INASM STACKMAN_LINKAGE_SWITCH
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
 * 1) store all platform and cpu state on the stack (callee-saved
 *    registers, exception state, etc)
 * 2) call the callback with the context, opcode STACKMAN_OP_SAVE and
 *    the current stack pointer. This allows the application to do additional
 *    cleanup and determine the new stack pointer.
 * 3) replace the stack pointer with the returned value from the callback.
 * 4) call the callback again with opcode STACKMAN_OP_RESTORE and the new
 *    stack pointer. This allows the application to initialize its state
      after the switch.
 * 5) Pop the platform state back from stack.
 * 6) return the result from the second call of the callback.
 *
 * Note that the callback and context are _not_ guaranteed
 * to be stack local.  That is, even if the stack pointer is modified
 * according to the first call to the callback, the same callback and
 * context may be used during the second callback, in the new stack.
 * Therefore, the same callback and context pointers should always be used
 * when switching between stacks.  This
 * restriction could be lifted by pushing both values onto the stack
 * before switching but the added complexity of platform implementations
 * doesn't seem to warrant that for such an exotic use case.
 *
 * an appropriate implementation is included by
 * platforms/platform.h
 * A template implementation is available in
 * platforms/switch_template.h
 */

/* Opcodes for the callback function */
typedef enum stackman_op_t {
	/* The callback receives the current stack pointer and should
	 * save any state that it wishes, for example the contents of the
	 * stack, before returning a new stack pointer.
	 */
	STACKMAN_OP_SAVE = 0,

	/* The callback receives the new stack pointer and should restore
	 * any state for it, e.g. filling the stack with the correct data.
	 * what it returns will be the return value from stackman_switch().
	 */
	STACKMAN_OP_RESTORE = 1,

	/* The callback is being invoked from a stackman_call() invokation.
	 * Stack_pointer is the pointer that was provided to stackman_call()
	 */
	STACKMAN_OP_CALL = 2,
	
} stackman_op_t;


/* The switch callback.  It receives the context passed to
 * stackman_switch(), an opcode (above) and a stack pointer.
 * The opcode is passed as an int, to avoid portability problems
 * with the ambiguous size of an enum.
 */
typedef void *(*stackman_cb_t)(
	void *context, int opcode, void *stack_pointer);
/* The actual stack switching function.
 * It saves state, switches stack pointer, and restores state
 */
STACKMAN_LINKAGE_SWITCH
void *stackman_switch(stackman_cb_t callback, void *context);


/* A function to call a function with a different stack location.
 * Instead of a normal function call, where the return address and arguments
 * are pushed on the current stack, first the stack pointer is changed to
 * a new location. This is useful for example to move the stack into heap memory.
 * 'callback' will be invoked with the
 * stack already set to the new position as specified by 'stack'. The third
 * argument to the callback is the *previous* stack pointer.
 * The callback can then proceed to 'save' the new stack for future switching.
 * Returning from the callback will work normally.
 */
STACKMAN_LINKAGE_SWITCH
void *stackman_call(stackman_cb_t callback, void *context, void *stack);


#endif /* STACKMAN_SWITCH_H */
