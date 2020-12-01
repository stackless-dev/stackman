/* This is the System V ABI for x86-64.
 * It is used on linux and other similar systems (other than
 * windows).
 * https://wiki.osdev.org/System_V_ABI and 
 * https://www.uclibc.org/docs/psABI-x86_64.pdf
 */

/* clang cannot perform inline assembly using specific __attr__
 * instructions, and so it may use a base pointer and other
 * things.  We must force it to use the pre-build assembler
 */

#if !defined(STACKMAN_ASSEMBLY_SRC)
#if defined (__clang__) || ! STACKMAN_INLINE_ASM
#define STACKMAN_ASSEMBLY_SRC "platforms/switch_x86_64_gcc.S"
#endif
#endif

#ifndef STACKMAN_HAVE_CALL
#define STACKMAN_HAVE_CALL 1
#define STACKMAN_STACK_ALIGN 16
#endif

#if defined(STACKMAN_SWITCH_IMPL)

#if !__ASSEMBLER__ && !defined(STACKMAN_ASSEMBLY_SRC)
/* inline assembly */
#include "../stackman_switch.h"

/* 
 * define registers to save.
 * x87 control word is callee saved and certain bits of the MXCSR
 * too, so we save both manually.
 */
#define PRESERVE "rbx", "r12", "r13", "r14", "r15", "rbp"
 
/* Need the optimization level to avoid storing 'stack_pointer'
 * and other locals
 * on the stack which would cause the wrong value to be sent to 
 * the second callback call (it would be read from stack).
 * We also enforce no frame pointer, so that locals are not
 * accessessed using rbp (base pointer), and push rbp ourselves.
 * stack protection code may be generated.  This is normally safe,
 * but can be forcefully disabled using "no-stack-protector" option.
 */
#define OPTIMIZE "O", "omit-frame-pointer", "no-stack-protector"
__attribute__((optimize(OPTIMIZE)))
STACKMAN_LINKAGE_SWITCH_INASM
void *STACKMAN_SWITCH_INASM_NAME(stackman_cb_t callback, void *context)
{
	void *stack_pointer;
	/* assembly to save non-volatile registers, including x87 and mmx */
	int mxcsr; short x87cw;
	__asm__ volatile (
		"fstcw %[cw]\n\t"
        "stmxcsr %[sr]\n\t"
		: [cw] "=m" (x87cw), [sr] "=m" (mxcsr) : : PRESERVE);
	
	/* sp = get stack pointer from assembly code */
	__asm__ ("movq %%rsp, %[result]" : [result] "=r" (stack_pointer));
	stack_pointer = callback(context, STACKMAN_OP_SAVE, stack_pointer);

	/* set stack pointer from sp using assembly */
	__asm__ ("movq %[result], %%rsp" :: [result] "r" (stack_pointer));

	stack_pointer = callback(context, STACKMAN_OP_RESTORE, stack_pointer);
	/* restore non-volatile registers from stack */
	__asm__ volatile (
        "ldmxcsr %[sr]\n\t"
        "fldcw %[cw]\n\t"
        : : [cw] "m" (x87cw), [sr] "m" (mxcsr));

	return stack_pointer;
}


/* 
 * Similar, but we want the frame pointer so that a debugger
 * can follow the stack
 */
#define OPTIMIZE_CALL "O", "no-omit-frame-pointer"
__attribute__((optimize(OPTIMIZE_CALL)))
STACKMAN_LINKAGE_SWITCH
void *stackman_call(stackman_cb_t callback, void *context, void *stack_pointer)
{
	void *old_sp, *result;
	/* sp = store stack pointer in rbx */
	__asm__ ("movq %%rsp, %%rbx" : : : "rbx");
	__asm__ ("movq %%rsp, %[sp]" : [sp] "=r" (old_sp));
	
	/* set stack pointer from provided using assembly */
	__asm__ ("movq %[sp], %%rsp" :: [sp] "r" (stack_pointer));

	result = callback(context, STACKMAN_OP_CALL, old_sp);
	/* restore stack pointer */
	__asm__ ("movq %%rbx, %%rsp" :::);
	
	return result;
}


#endif

#if __ASSEMBLER__ && defined(STACKMAN_ASSEMBLY_SRC)
/* pre-generated assembly code */
#include STACKMAN_ASSEMBLY_SRC
#endif

#endif /* STACKMAN_SWITCH_IMPL */
