/* Switch function for Aarch64 and GCC compiler
 * for AAPCS64 abi, see https://developer.arm.com/documentation/ihi0055/b/
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
#define STACKMAN_EXTERNAL_ASM "platforms/switch_aarch64_gcc.S"
#endif
#endif

#define STACKMAN_HAVE_CALL 1
#define STACKMAN_STACK_ALIGN 16

#ifdef STACKMAN_SWITCH_IMPL
#if !__ASSEMBLER__ && !defined(STACKMAN_EXTERNAL_ASM)

/* 
 * To test this, #include this file in a file, test.c and
 * gcc -S -DSTACKMAN_SWITCH_IMPL test.c
 * then examine test.s for the result.
 * We instruct optimizer to not omit frame pointers, -fno-omit_frame_pointer
 * and not use local stack vars, -O1.
 * option is applied with an __attribute__.
 */

#include "../stackman_switch.h"

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
STACKMAN_LINKAGE_SWITCH
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

/* 
 * Similar, but we want the frame pointer so that a debugger
 * can follow the stack.  x19 is callee saved local var register.
 */
__attribute__((optimize("O", "no-omit-frame-pointer")))
STACKMAN_LINKAGE_SWITCH
void *stackman_call(stackman_cb_t callback, void *context, void *stack_pointer)
{
  void *sp;
  /* sp = store stack pointer in rbx */
  __asm__ ("movl x19, sp" : : : "x19");

  /* set stack pointer from provided using assembly */
  __asm__ ("movl sp, %[result]" :: [result] "r" (stack_pointer));

  sp = callback(context, STACKMAN_OP_CALL, stack_pointer);
  /* restore stack pointer */
  __asm__ ("movl sp, x19" :::);
  
  return sp;
}

#if __ASSEMBLER__ && defined(STACKMAN_EXTERNAL_ASM)
/* pre-generated assembly code */
#include STACKMAN_EXTERNAL_ASM
#endif

#endif /* STACKMAN_SWITCH_IMPL */
