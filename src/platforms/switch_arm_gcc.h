/* Switch function for arm 32 as seen on RaspberryPi2
 * for ARM abi (AAPCS), see https://developer.arm.com/documentation/ihi0042/e/
 * and additional information for gcc inline arm assembly:
 * http://www.ethernut.de/en/documents/arm-inline-asm.html
 *
 */

/* clang cannot perform inline assembly using specific __attr__
 * instructions, and so it may use a base pointer and other
 * things.  We must force it to use the pre-build assembler
 */
#if !defined(STACKMAN_EXTERNAL_ASM)
#if defined (__clang__) || STACKMAN_PREFER_ASM
#define STACKMAN_EXTERNAL_ASM "platforms/switch_arm_gcc.S"
#endif
#endif

#define STACKMAN_HAVE_CALL 1
#define STACKMAN_STACK_ALIGN 8

#ifdef STACKMAN_SWITCH_IMPL
#if !__ASSEMBLER__ && !defined(STACKMAN_EXTERNAL_ASM)

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
 * pointer, so we simply use that method. the -fomit-frame-pointer
 * option is applied with an __attribute__, and r11 explicitly pushed.
 * we must also enable optimization level "O" so that intermediates
 * aren't stored on the stack.
 */

#include "../stackman_switch.h"

/* these are the core registers that must be preserved. We save them because
 * we have no idea what happens after the switch, and the caller of this function
 * assumes that they are left in place when we return to him.
 * If frame pointers are geing generated, gcc disallows us to use r11
 * since it uses it for that.  It will also restore sp on exit using
 * r11. But disabling frame pointers for this function allows us to
 * push it normally and restore it.
 */
#define NV_REGISTERS "r4", "r5","r6", "r7", "r8", "r9", "r10" , "r11"

/* These are the floating point extension registers. Same applies, we must preserve them in
 * case the calling function was doing any floating point logic.
 * it may be optionally disabled to store these floating point registers by not
 * defining them here.
 * Note that we do not
 * preserve the FPSCR and VPR registers since they have complex rules about preservation.
 */
#ifndef __thumb__
#define CP_REGISTERS "d8","d9","d10","d11","d12","d13","d14","d15"
#endif

__attribute__((optimize("O", "omit-frame-pointer")))
STACKMAN_LINKAGE_SWITCH
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

/* 
 * Similar, but we want the base pointer so that a debugger
 * can follow the stack
 */
__attribute__((optimize("O", "no-omit-frame-pointer")))
STACKMAN_LINKAGE_SWITCH
void *stackman_call(stackman_cb_t callback, void *context, void *stack_pointer)
{
  void *old_sp, *result;
  /* sp = store stack pointer in r4, a callee-saved scratch register */
  __asm__ ("mov r4, sp" : : : "r4");
  __asm__ ("mov %[var], sp" : [var] "=r" (old_sp));

  /* set stack pointer from provided using assembly */
  __asm__ ("mov sp, %[var]" :: [var] "r" (stack_pointer));

  result = callback(context, STACKMAN_OP_CALL, old_sp);
  /* restore stack pointer */
  __asm__ ("mov sp, r4" :::);
  
  return result;
}

#endif

#if __ASSEMBLER__ && defined(STACKMAN_EXTERNAL_ASM)
/* pre-generated assembly code */
#include STACKMAN_EXTERNAL_ASM
#endif

#endif /* STACKMAN_SWITCH_IMPL */
