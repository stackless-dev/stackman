/* Switch function for Aarch64 and GCC compiler
 * for Aarch64 abi, see
 * https://developer.arm.com/docs/ihi0055/d/procedure-call-standard-for-the-arm-64-bit-architecture
 * and additional information for gcc inline arm assembly:
 * http://www.ethernut.de/en/documents/arm-inline-asm.html
 *
 */
#ifdef STACKMAN_SWITCH_IMPL
#if ! STACKMAN_SWITCH_ASM

#ifndef USE_ASSEMBLER
/* 
 * To test this, #include this file in a file, test.c and
 * gcc -S -DSTACKMAN_SWITCH_IMPL test.c
 * then examine test.s for the result.
 * We instruct optimizer to not omit frame pointers, -fno-omit_frame_pointer
 * and not use local stack vars, -O1.
 * option is applied with an __attribute__.
 */

#undef STACKMAN_SWITCH_IMPL
#include "../stackman.h"

/* these are the core registers that must be preserved. We save them because
 * we have no idea what happens after the switch, and the caller of this function
 * assumes that they are left in place when we return to him.
 * To make sure the frame pointer (x29) is pushed to stack, we force
 * optimizer to not omit frame pointer.
 */
#define GP_PRESERVE "x19", "x20","x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28"

/* lower 64 bits of v8-v15 must be preserved */
#define FP_PRESERVE "d8","d9","d10","d11","d12","d13","d14","d15"


__attribute__((optimize("O1", "no-omit-frame-pointer")))
void *stackman_switch(stackman_cb_t callback, void *context)
{
	void *sp;
	/* have gcc save/restore registers for us on the stack */
	__asm__ volatile ("" : : : GP_PRESERVE);
#ifdef FP_PRESERVE
	__asm__ volatile ("" : : : FP_PRESERVE);
#endif
	/* sp = get stack pointer from assembly code */
	__asm__ ("mov %[result], sp" : [result] "=r" (sp));
	/* store stack */
	sp = callback(context, STACKMAN_OP_SAVE, sp);
	/* set stack pointer from sp using assembly */
	__asm__ ("mov sp, %[result]" : : [result] "r" (sp));
	sp = callback(context, STACKMAN_OP_RESTORE, sp);
	return sp;
}
#endif
#else
/* assembler code here, if the above cannot be done in in-line assembly */
#endif
#endif
