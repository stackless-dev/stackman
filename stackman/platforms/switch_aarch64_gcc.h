/* Switch function for Aarch64 and GCC compiler
 * for AAPCS64 abi, see https://developer.arm.com/documentation/ihi0055/b/
 * and additional information for gcc inline arm assembly:
 * http://www.ethernut.de/en/documents/arm-inline-asm.html
 *
 * stackman_switch stores FPCR and FPSR in the stack image and writes
 * them back after the restore callback. A thread switch saves both,
 * including the caller-saved status bits. stackman_call does not
 * change them and does not spill them.
 *
 * The same slot carries the caller's TPIDR2 block pointer so a dormant
 * SME ZA state can be committed on the way out and restored on the way
 * back in. See docs/scalable-matrix-extension.md. stackman_call does
 * not touch it: that call returns to the same caller.
 *
 */
/* clang cannot perform inline assembly using specific __attr__
 * instructions, and so it may use a base pointer and other
 * things.  We must force it to use the pre-build assembler
 */
#if !defined(STACKMAN_ASSEMBLY_SRC)
#if defined (__clang__) || ! STACKMAN_INLINE_ASM
#define STACKMAN_ASSEMBLY_SRC "platforms/switch_aarch64_gcc.S"
#endif
#endif

#ifndef STACKMAN_HAVE_CALL
#define STACKMAN_HAVE_CALL 1
#define STACKMAN_STACK_ALIGN 16
#endif

#ifdef STACKMAN_SWITCH_IMPL

#if !__ASSEMBLER__
/*
 * Called from switch_aarch64_gcc.S and from the inline copy below.
 * Weak on purpose: a toolchain without the AAPCS64 SME runtime resolves
 * these to zero, and the switch stores a null pointer.
 *
 * target("arch=armv8-a") keeps an SME-aware compiler from wrapping the
 * helpers in a lazy save of its own. smstart za is encoded with .inst so
 * this file still assembles for plain armv8-a. 0xd503457f is "smstart za".
 * S3_3_C13_C0_5 is TPIDR2_EL0.
 */
extern void __arm_za_disable(void) __attribute__((weak));
extern void __arm_tpidr2_restore(void *) __attribute__((weak));

typedef struct {
	unsigned long long status;
	void *tpidr2;
} stackman_sme_state_result;

extern stackman_sme_state_result __arm_sme_state(void) __attribute__((weak));

__attribute__((visibility("hidden"), noinline, target("arch=armv8-a")))
void *stackman_sme_suspend(void);

__attribute__((visibility("hidden"), noinline, target("arch=armv8-a")))
void stackman_sme_resume(void *tpidr2);

void *stackman_sme_suspend(void)
{
	stackman_sme_state_result state;

	if (!__arm_sme_state || !__arm_za_disable || !__arm_tpidr2_restore)
		return 0;
	state = __arm_sme_state();
	if (!state.tpidr2)
		return 0;
	__arm_za_disable();
	return state.tpidr2;
}

void stackman_sme_resume(void *tpidr2)
{
	if (!tpidr2 || !__arm_tpidr2_restore)
		return;
	{
		/* x0 survives __arm_tpidr2_restore. The helper preserves it. */
		register void *blk asm("x0") = tpidr2;
		register void (*fn)(void *) asm("x16") = __arm_tpidr2_restore;

		__asm__ volatile(
			".inst 0xd503457f\n\t"
			"blr %1\n\t"
			"msr S3_3_C13_C0_5, %0\n\t"
			: "+r"(blk)
			: "r"(fn)
			: "x14", "x15", "x17", "x30", "memory", "cc");
	}
}

#if defined(__APPLE__)
#define STACKMAN_SME_RESUME "bl _stackman_sme_resume\n\t"
#else
#define STACKMAN_SME_RESUME "bl stackman_sme_resume\n\t"
#endif
#endif /* !__ASSEMBLER__ */

#if !__ASSEMBLER__ && !defined(STACKMAN_ASSEMBLY_SRC)

/* 
 * To test this, compile with appropriate cross platform flags, e.g.
 * "arm-linux-gnueabi-gcc -S gen_asm.c"
 * then examine gen_asm.s for the result.
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
STACKMAN_LINKAGE_SWITCH_INASM
void *STACKMAN_SWITCH_INASM_NAME(stackman_cb_t callback, void *context)
{
	void *sp;
	void *tpidr2;
	/* have gcc save/restore registers for us on the stack */
	__asm__ volatile ("" : : : GP_PRESERVE);
#ifdef FP_PRESERVE
	__asm__ volatile ("" : : : FP_PRESERVE);
#endif
	/* FPCR and FPSR are one thread context on Linux and macOS.
	 * Park both in the bottom of a 32-byte slot and pass that sp
	 * to the callback, so the copied image carries them. The next
	 * word is the TPIDR2 block pointer just captured, or null when
	 * ZA was off or the SME helpers are absent. x1 and x2 are free
	 * until the compiler assigns the saved stack pointer.
	 */
	tpidr2 = stackman_sme_suspend();
	__asm__ volatile (
		"mrs x1, fpcr\n\t"
		"mrs x2, fpsr\n\t"
		"stp x1, x2, [sp, #-32]!\n\t"
		"stp %0, xzr, [sp, #16]"
		:: "r"(tpidr2) : "x1", "x2", "memory");
	/* The compiler's CFI still describes its own frame. This
	 * adjusts by the slot just pushed. It is not in the stp asm:
	 * that string is copied into the shared .S, and Darwin
	 * assembles the copy with no .cfi_startproc. The .S sets the
	 * CFA with CFI_DEF_CFA_OFFSET instead.
	 */
#if !defined(__APPLE__)
	__asm__ volatile (".cfi_adjust_cfa_offset 32");
#endif
	/* sp = get stack pointer from assembly code */
	__asm__ ("mov %[result], sp" : [result] "=r" (sp));
	/* store stack */
	sp = callback(context, STACKMAN_OP_SAVE, sp);
	/* Keep the live frame pointer attached to the new stack.
	 * x29 is not an asm clobber: a clobber would make gcc save
	 * and restore it around this block and undo the delta.
	 * Resume a dormant ZA before the callback runs on this stack.
	 * A null slot means the entry mode was off. The call is inside
	 * this asm so the compiler does not spill between the sp write
	 * and the restore.
	 */
	__asm__ volatile (
		"sub x1, sp, %[result]\n\t"
		"mov sp, %[result]\n\t"
		"sub x29, x29, x1\n\t"
		"ldr x0, [sp, #16]\n\t"
		"cbz x0, .Lsme%=\n\t"
		STACKMAN_SME_RESUME
		".Lsme%=:\n\t"
		: : [result] "r" (sp)
		: "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
		  "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
		  "x16", "x17", "x30",
		  "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
		  "memory", "cc");
	sp = callback(context, STACKMAN_OP_RESTORE, sp);
	/* Reload FPCR and FPSR from the stack now current, then release
	 * the slot before the epilogue pops the callee-saved registers.
	 */
	__asm__ volatile (
		"ldp x1, x2, [sp], #32\n\t"
		"msr fpcr, x1\n\t"
		"msr fpsr, x2"
		::: "x1", "x2", "memory");
#if !defined(__APPLE__)
	__asm__ volatile (".cfi_adjust_cfa_offset -32");
#endif
	return sp;
}

/* 
 * Similar, but we want the frame pointer so that a debugger
 * can follow the stack.  x19 is callee saved local var register.
 */
__attribute__((optimize("O", "no-omit-frame-pointer")))
STACKMAN_LINKAGE_SWITCH
void *stackman_call(stackman_cb_t callback, void *context, void *stack_pointer)
{
  void *old_sp, *result;
  /* sp = store stack pointer in x19, a callee-saved scratch register. */
  __asm__ ("mov x19, sp" : : : "x19");
  __asm__ ("mov %[var], sp" : [var] "=r" (old_sp));


  /* set stack pointer from provided using assembly */
  if(stack_pointer != 0)
  	__asm__ ("mov sp, %[var]" :: [var] "r" (stack_pointer));

  result = callback(context, STACKMAN_OP_CALL, old_sp);
  /* restore stack pointer */
  __asm__ ("mov sp, x19" :::);
  
  return result;
}

#endif

#if __ASSEMBLER__ && defined(STACKMAN_ASSEMBLY_SRC)
/* pre-generated assembly code */
#include STACKMAN_ASSEMBLY_SRC
#endif

#endif /* STACKMAN_SWITCH_IMPL */
