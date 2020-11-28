
/* Template function for the switch
 * Requires in-line assembly to save/restore registers from stack
 * and to get and restore the stack pointer.
 * for example use, see switch_arm_gcc.h or switch_x86_64_gcc.h
 * A single callback is used both for saving and restoring stack,
 * and it maintains state inside its context storage.
 * This simplifies the assembly code which has no branching.
 */

/* clang cannot perform inline assembly using specific __attr__
 * instructions, and so it may use a base pointer and other
 * things.  We must force it to use the pre-build assembler
 */
#if !defined(STACKMAN_ASSEMBLY_SRC)
#if defined (__clang__) || ! STACKMAN_INLINE_ASM
#define STACKMAN_ASSEMBLY_SRC "platforms/switch_xxx.S"
#endif
#endif


#if defined(STACKMAN_SWITCH_IMPL )
#if !__ASSEMBLER__ && !defined(STACKMAN_ASSEMBLY_SRC)

#include "../stackman_switch.h"

void *stackman_switch(stackman_cb_t callback, void *context)
{
	void *stack_pointer;
	/* assembly to save non-volatile registers
     * those, according to abi, that functions must save/restore if they
     * intend to use them
	/* __asm__("push volatile registers") */

	/* sp = get stack pointer from assembly code */
	/* __asm__("get sp into stack_pointer") */
	stack_pointer = callback(context, STACKMAN_OP_SAVE, stack_pointer);

	/* set stack pointer from sp using assembly */
	/* __asm__("store sp from stack_pointer") */
	stack_pointer = callback(context, STACKMAN_OP_RESTORE, stack_pointer);
	/* restore non-volatile registers from stack */
	/* __asm__("pop volatile registers") */
	return stack_pointer;
}
#endif

#if __ASSEMBLER__ && defined(STACKMAN_ASSEMBLY_SRC)
/* pre-generated assembly code */
#include STACKMAN_ASSEMBLY_SRC
#endif

#endif /* STACKMAN_SWITCH_IMPL */