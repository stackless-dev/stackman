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
	.file	"gen_asm.c"
	.text
	.globl	stackman_switch
	.type	stackman_switch, @function
stackman_switch:
.LFB0:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$24, %rsp
	.cfi_def_cfa_offset 80
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
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE0:
	.size	stackman_switch, .-stackman_switch
	.globl	stackman_call
	.type	stackman_call, @function
stackman_call:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$8, %rsp
	.cfi_offset 3, -24
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
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	stackman_call, .-stackman_call
	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
