#pragma once
#ifndef STACKMAN_SWITCH_H
#define STACKMAN_SWITCH_H

#ifdef STACKMAN_SWITCH_STATIC
#define STACKMAN_SWITCH_STORAGE static
#else
#define STACKMAN_SWITCH_STORAGE
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
 * 1) store all platform and cpu state on the stack (callee-stored
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
 * context will be used during the second callback, in the new stack * Thus, both stacks must really belong to the same program.  This
 * restriction could be lifted by pushing both values onto the stack
 * before switching but the added complexity of platform implementations
 * doesn't seem to warrant that for such an exotic use case.
 *
 * an appropriate implementation is included b
 * platf_tealet/tealet_platformselect.h
 * A template implementation is available in
 * platf_tealet/switch_template.h
 */

/* Opcodes for the callback function */
typedef enum stackman_op_t {
	/* The callback receives the current stack pointer and should
	 * save any state that it wishes, for example the contents of the
	 * stack, before returning a new stack pointer.
	 */
	STACKMAN_OP_SAVE = 0,

	/* The callback receives the new stack pointer and should restore
	 * any state for it, e.g. fillint the stack with the correct data.
	 * what it returns will be the return value from stackman_switch().
	 */
	STACKMAN_OP_RESTORE = 1,
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
STACKMAN_SWITCH_STORAGE
void *stackman_switch(stackman_cb_t callback, void *context);

/* a separate function which wraps the c function with in-line assembly
 * in a way to ensure tht it won't be inlined in other code.
 * This is a temporary hack until we can do it more nicely
 */
void *stackman_switch_noinline(stackman_cb_t callback, void *context);


#endif /* STACKMAN_SWITCH_H */