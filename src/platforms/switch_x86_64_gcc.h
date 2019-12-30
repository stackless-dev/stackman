/* This is the System V ABI for x86-64.
 * It is used on linux and other similar systems (other than
 * windows).
 * https://wiki.osdev.org/System_V_ABI
 * Note that we don't preserve the mmx mxcsr register or the
 * fp x87 cw (control word) as is strictly required by the ABI
 * since it requires more custom assembly.  switching between
 * floating point functions is therefore dangerous.
 */

#if defined STACKMAN_SWITCH_IMPL && !STACKMAN_SWITCH_ASM
#undef STACKMAN_SWITCH_IMPL
#include "../stackman.h"

#define PRESERVE "rbx", "r12", "r13", "r14", "r15"

/* Need the optimization level to avoid storing 'stack_pointer'
 * and other locals
 * on the stack which would cause the wrong value to be sent to 
 * the second callback call (it would be read from stack).
 * Also, ensure a frame pointer is made, pushing rbp.
 */
__attribute__((optimize("O", "no-omit-frame-pointer")))
void *stackman_switch(stackman_cb_t callback, void *context)
{
	void *stack_pointer;
	/* assembly to save non-volatile registers */
	__asm__ volatile ("" : : : PRESERVE);
	/* sp = get stack pointer from assembly code */
	__asm__ ("movq %%rsp, %[result]" : [result] "=r" (stack_pointer));
	stack_pointer = callback(context, STACKMAN_OP_SAVE, stack_pointer);

	/* set stack pointer from sp using assembly */
	__asm__ ("movq %[result], %%rsp" :: [result] "r" (stack_pointer));

	stack_pointer = callback(context, STACKMAN_OP_RESTORE, stack_pointer);
	/* restore non-volatile registers from stack */
	return stack_pointer;
}
#endif
