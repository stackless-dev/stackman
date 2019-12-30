/* Switch function for arm 32 as seen on RaspberryPi2
 * for ARM abi, see http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ihi0042f/index.html
 * and additional information for gcc inline arm assembly:
 * http://www.ethernut.de/en/documents/arm-inline-asm.html
 *
 */
#ifdef STACKMAN_SWITCH_IMPL
#if ! STACKMAN_SWITCH_ASM

#ifndef USE_ASSEMBLER
/* inline assembly does not _by default_ produce code that is usable
 * because depending
 * on optimization,the fp (r11) register may be used to restore the
 * stack pointer on function exit, thereby invalidating our changes
 * to the stack pointer.
 * To arrive at reasonable assembler, follow some approach similar to:
 * cp switch_arm_gcc.h test.c
 * gcc -S -O -DSTACKMAN_SWITCH_IMPL test.c
 * mv test.s switch_arm_gcc.s
 * assembly code which can then be modified for actual use.  Simple optimized
 * version is better than no-optimized because the latter uses stack
 * variables for arguments.  And it turns out that this version actually
 * runs because it does not use fp for restoring the stack pointer
 * 
 * However, for now the optimizer can be instructed to omit frame
 * ponter, so we simply use that method. the -fomit-frame-pointer
 * option is applied with an __attribute__.
 * we must also enable optimization level "O" so that intermediates
 * aren't stored on the stack.
 */

#undef STACKMAN_SWITCH_IMPL
#include "../stackman.h"

/* these are the core registers that must be preserved. We save them because
 * we have no idea what happens after the switch, and the caller of this function
 * assumes that they are left in place when we return to him.
 * r11 is not explicitly mentioned, gcc uses it as fp and pushes it
 * implicitly.
 */
#define NV_REGISTERS "r4", "r5","r6", "r7", "r8", "r9", "r10" /*, "r11"*/
/* These are the floating point extension registers. Same applies, we must preserve them in
 * case the calling function was doing any floating point logic.  Note that we do not
 * preserve the FPSCR and VPR registers since they have complex rules about preservation.
 * it may be optionally disabled to store these floating point registers by not
 * defining them here.
 */
#ifndef __thumb__
#define CP_REGISTERS "d8","d9","d10","d11","d12","d13","d14","d15"
#endif

__attribute__((optimize("O", "omit-frame-pointer")))
void *stackman_switch(stackman_cb_t callback, void *context)
{
	void *sp;
	__asm__ volatile ("" : : : NV_REGISTERS);
#ifdef CP_REGISTERS
	__asm__ volatile ("" : : : CP_REGISTERS);
#endif
	/* assembly to save non-volatile registers
         * those, according to abi, that functions must save/restore if they
         * intend to use them
	*/
	/* sp = get stack pointer from assembly code */
	__asm__ ("mov %[result], sp" : [result] "=r" (sp));
	/* store stack */
	sp = callback(context, STACKMAN_OP_SAVE, sp);
	/* set stack pointer from sp using assembly */
	__asm__ ("mov sp, %[result]" : : [result] "r" (sp));
	sp = callback(context, STACKMAN_OP_RESTORE, sp);
	/* restore registers */
	return sp;
}
#endif
#else
/* assembler code here, if the above cannot be done in in-line assembly */
#endif
#endif
