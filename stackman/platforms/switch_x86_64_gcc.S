/*
 * This file is provided for where inline assembly is not
 * dependable, such as with clang on linux x86_64 or if
 * STACKMAN_PREFER_ASM is defined. It provides a stable
 * implementation that is independent of subtleties of inline
 * assembly code which may change with compiler versions.
 * The file is generated using
 * "gcc -S -fcf-protection=none gen_asm.c"
 * and then copying the code from gen_asm.s into this file.
 * 
 */

/* Mach-O (macOS) vs ELF (Linux) compatibility macros */
#ifdef __APPLE__
#define FUNCTION(name) .globl _##name
#define LABEL(name) _##name:
#define TYPE_FUNCTION(name)
#define SIZE_FUNCTION(name)
#define GNU_STACK
/* Disable CFI directives on macOS - they have different semantics */
#define CFI_STARTPROC
#define CFI_ENDPROC
#define CFI_DEF_CFA_OFFSET(x)
#define CFI_DEF_CFA_REGISTER(x)
#define CFI_OFFSET(r,o)
#define CFI_RESTORE(r)
#else
#define FUNCTION(name) .globl name ; .type name, @function
#define LABEL(name) name:
#define TYPE_FUNCTION(name) .type name, @function
#define SIZE_FUNCTION(name) .size name, .-name
#define GNU_STACK .section .note.GNU-stack,"",@progbits
#define CFI_STARTPROC .cfi_startproc
#define CFI_ENDPROC .cfi_endproc
#define CFI_DEF_CFA_OFFSET(x) .cfi_def_cfa_offset x
#define CFI_DEF_CFA_REGISTER(x) .cfi_def_cfa_register x
#define CFI_OFFSET(r,o) .cfi_offset r, o
#define CFI_RESTORE(r) .cfi_restore r
#endif

	.file	"gen_asm.c"
	.text
	FUNCTION(stackman_switch)
LABEL(stackman_switch)
.LFB0:
	CFI_STARTPROC
	pushq	%r15
	CFI_DEF_CFA_OFFSET(16)
	CFI_OFFSET(15,-16)
	pushq	%r14
	CFI_DEF_CFA_OFFSET(24)
	CFI_OFFSET(14,-24)
	pushq	%r13
	CFI_DEF_CFA_OFFSET(32)
	CFI_OFFSET(13,-32)
	pushq	%r12
	CFI_DEF_CFA_OFFSET(40)
	CFI_OFFSET(12,-40)
	pushq	%rbp
	CFI_DEF_CFA_OFFSET(48)
	CFI_OFFSET(6,-48)
	pushq	%rbx
	CFI_DEF_CFA_OFFSET(56)
	CFI_OFFSET(3,-56)
	subq	$24, %rsp
	CFI_DEF_CFA_OFFSET(80)
	movq	%rdi, %rax
	movq	%rsi, %rcx
#APP
# 56 "../platforms/switch_x86_64_gcc.h" 1
	fstcw 10(%rsp)
	stmxcsr 12(%rsp)
	
# 0 "" 2
# 62 "../platforms/switch_x86_64_gcc.h" 1
	movq %rsp, %rbx
# 0 "" 2
#NO_APP
	movq	%rbx, %rdx
	movl	$0, %esi
	movq	%rcx, %r15
	movq	%rcx, %rdi
	movq	%rax, %r14
	call	*%rax
#APP
# 67 "../platforms/switch_x86_64_gcc.h" 1
	movq %rax, %rsp
# 0 "" 2
#NO_APP
	subq	%rbx, %rax
#APP
# 68 "../platforms/switch_x86_64_gcc.h" 1
	addq %rax,  %rbp
# 0 "" 2
#NO_APP
	movq	%rbx, %rdx
	movl	$1, %esi
	movq	%r15, %rdi
	call	*%r14
#APP
# 72 "../platforms/switch_x86_64_gcc.h" 1
	ldmxcsr 12(%rsp)
	fldcw 10(%rsp)
	
# 0 "" 2
#NO_APP
	addq	$24, %rsp
	CFI_DEF_CFA_OFFSET(56)
	popq	%rbx
	CFI_DEF_CFA_OFFSET(48)
	popq	%rbp
	CFI_DEF_CFA_OFFSET(40)
	popq	%r12
	CFI_DEF_CFA_OFFSET(32)
	popq	%r13
	CFI_DEF_CFA_OFFSET(24)
	popq	%r14
	CFI_DEF_CFA_OFFSET(16)
	popq	%r15
	CFI_DEF_CFA_OFFSET(8)
	ret
	CFI_ENDPROC
.LFE0:
	SIZE_FUNCTION(stackman_switch)
	FUNCTION(stackman_call)
LABEL(stackman_call)
.LFB1:
	CFI_STARTPROC
	pushq	%rbp
	CFI_DEF_CFA_OFFSET(16)
	CFI_OFFSET(6,-16)
	movq	%rsp, %rbp
	CFI_DEF_CFA_REGISTER(6)
	pushq	%rbx
	subq	$8, %rsp
	CFI_OFFSET(3,-24)
	movq	%rdi, %rax
	movq	%rsi, %rdi
#APP
# 92 "../platforms/switch_x86_64_gcc.h" 1
	movq %rsp, %rbx
# 0 "" 2
# 93 "../platforms/switch_x86_64_gcc.h" 1
	movq %rsp, %rcx
# 0 "" 2
#NO_APP
	testq	%rdx, %rdx
	je	.L4
#APP
# 97 "../platforms/switch_x86_64_gcc.h" 1
	movq %rdx, %rsp
# 0 "" 2
#NO_APP
.L4:
	movq	%rcx, %rdx
	movl	$2, %esi
	call	*%rax
#APP
# 101 "../platforms/switch_x86_64_gcc.h" 1
	movq %rbx, %rsp
# 0 "" 2
#NO_APP
	addq	$8, %rsp
	popq	%rbx
	popq	%rbp
	CFI_DEF_CFA_OFFSET(8)
	ret
	CFI_ENDPROC
.LFE1:
	SIZE_FUNCTION(stackman_call)
	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
	GNU_STACK
