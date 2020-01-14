/* This is the System V ABI for x86-64.
 * It is used on linux and other similar systems (other than
 * windows).
 * https://wiki.osdev.org/System_V_ABI and 
 * https://www.uclibc.org/docs/psABI-x86_64.pdf
 */

/* clang cannot perform inline assembly using specific __attr__
 * instructions, and so it may use a base pointer and other
 * things.  We must force it to use the pre-build assembler
 * which has been generated previously with a gcc -S pass
 */

#if !defined(STACKMAN_EXTERNAL_ASM)
#if defined (__clang__) || defined(STACKMAN_SWITCH_ASM_TEST)
#define STACKMAN_EXTERNAL_ASM "platforms/switch_x86_64_gcc.S"
#endif
#endif

#if defined(STACKMAN_SWITCH_IMPL)

#if !defined(STACKMAN_EXTERNAL_ASM) && !STACKMAN_SWITCH_ASM
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
void *stackman_switch(stackman_cb_t callback, void *context)
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

#endif

#if STACKMAN_SWITCH_ASM && defined(STACKMAN_EXTERNAL_ASM)
/* pre-generated assembly code */
#include STACKMAN_EXTERNAL_ASM
#endif

#endif /* STACKMAN_SWITCH_IMPL */
