/* Switch function for 32-bit ARM hard-float (AAPCS-VFP, gnueabihf).
 *
 * d8-d15 are callee-saved. The empty asm clobber makes gcc spill them
 * in the prologue, before the stack pointer is passed to the callback,
 * so a copied stack image carries them. stackman_call does not touch
 * them and does not spill them.
 *
 * stackman_switch also stores the whole FPSCR in that image and writes
 * it back after the restore callback. A thread switch does the same.
 * The store includes the caller-saved flag bits. stackman_call does
 * not change FPSCR and does not spill it.
 *
 * Do not adjust r11: stackman_switch is built with omit-frame-pointer,
 * so r11 is not this frame.
 *
 * Softfp (-mfloat-abi=softfp) does not define __ARM_PCS_VFP, so it
 * does not use this file.
 *
 * Generate the checked-in assembly with:
 *   arm-linux-gnueabihf-gcc -S -marm gen_asm.c
 */

#if !defined(__ARM_PCS_VFP)
#error This ARM32 assembly spills d8-d15 and is only for hard-float (__ARM_PCS_VFP).
#endif

/* clang cannot perform inline assembly using specific __attr__
 * instructions, and so it may use a base pointer and other
 * things.  We must force it to use the pre-built assembler
 */
#if !defined(STACKMAN_ASSEMBLY_SRC)
#if defined (__clang__) || ! STACKMAN_INLINE_ASM
#define STACKMAN_ASSEMBLY_SRC "platforms/switch_armhf_gcc.S"
#endif
#endif

#ifndef STACKMAN_STACK_ALIGN
#define STACKMAN_STACK_ALIGN 8
#endif

#ifdef STACKMAN_SWITCH_IMPL
#if !__ASSEMBLER__ && !defined(STACKMAN_ASSEMBLY_SRC)

/* omit-frame-pointer: without it, gcc can restore sp from r11 on exit
 * and undo the switch. -O keeps the arguments in registers instead of
 * stack slots. Regenerate the checked-in assembly with the command in
 * the file header.
 */

#include "../stackman_switch.h"

/* these are the core registers that must be preserved. We save them because
 * we have no idea what happens after the switch, and the caller of this function
 * assumes that they are left in place when we return to him.
 * If frame pointers are being generated, gcc disallows us to use r11
 * since it uses it for that.  It will also restore sp on exit using
 * r11. But disabling frame pointers for this function allows us to
 * push it normally and restore it.
 */
#define NV_REGISTERS "r4", "r5","r6", "r7", "r8", "r9", "r10" , "r11"

/* AAPCS-VFP callee-saved doubles. The checked-in .S is generated from
 * this clobber, so the prologue spill is part of that file too.
 */
#define CP_REGISTERS "d8","d9","d10","d11","d12","d13","d14","d15"

__attribute__((optimize("O", "omit-frame-pointer")))
STACKMAN_LINKAGE_SWITCH_INASM
void *STACKMAN_SWITCH_INASM_NAME(stackman_cb_t callback, void *context)
{
	void *sp;
	__asm__ volatile ("" : : : NV_REGISTERS);
	__asm__ volatile ("" : : : CP_REGISTERS);
	/* assembly to save non-volatile registers
         * those, according to abi, that functions must save/restore if they
         * intend to use them
	*/
	/* Park the whole FPSCR below sp. The eight-byte slot keeps the
	 * AAPCS alignment, and it is inside the image the callback copies.
	 * r2 is free until the compiler assigns the saved stack pointer.
	 */
	__asm__ volatile (
		"vmrs r2, fpscr\n\t"
		"str r2, [sp, #-8]!"
		::: "r2", "memory");
	/* sp = get stack pointer from assembly code */
	__asm__ ("mov %[result], sp" : [result] "=r" (sp));
	/* store stack */
	sp = callback(context, STACKMAN_OP_SAVE, sp);
	/* set stack pointer from sp using assembly */
	__asm__ ("mov sp, %[result]" : : [result] "r" (sp));
	sp = callback(context, STACKMAN_OP_RESTORE, sp);
	/* Reload FPSCR from the stack now current, then release the slot
	 * before the epilogue pops d8-d15.
	 */
	__asm__ volatile (
		"ldr r2, [sp], #8\n\t"
		"vmsr fpscr, r2"
		::: "r2", "memory");
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
  if (stack_pointer != 0)
  	__asm__ ("mov sp, %[var]" :: [var] "r" (stack_pointer));

  result = callback(context, STACKMAN_OP_CALL, old_sp);
  /* restore stack pointer */
  __asm__ ("mov sp, r4" :::);

  return result;
}

#endif

#if __ASSEMBLER__ && defined(STACKMAN_ASSEMBLY_SRC)
/* pre-generated assembly code */
#include STACKMAN_ASSEMBLY_SRC
#endif

#endif /* STACKMAN_SWITCH_IMPL */
