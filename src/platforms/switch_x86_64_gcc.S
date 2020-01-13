/*
 * this file is provided as a test for the pure
 * assembly include mechanism, on linux x86_64 
 * 
 */
	.file	"switch_x86_64_gcc.S"
	.text
	.globl	stackman_switch
	.type	stackman_switch, @function
stackman_switch:
.LFB0:
	.cfi_startproc
	endbr64
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
	movq	%rdi, %r8
	movq	%rsi, %rdi
	movl	$40, %eax
	movq	%fs:(%rax), %rsi
	movq	%rsi, 8(%rsp)
	xorl	%esi, %esi
#APP
# 39 "switch_x86_64_gcc.h" 1
	fstcw 2(%rsp)
	stmxcsr 4(%rsp)
	
# 0 "" 2
# 44 "switch_x86_64_gcc.h" 1
	movq %rsp, %rdx
# 0 "" 2
#NO_APP
	movq	%rdi, %r14
	movq	%r8, %rbx
	call	*%r8
	movq	%rax, %rdx
#APP
# 48 "switch_x86_64_gcc.h" 1
	movq %rax, %rsp
# 0 "" 2
#NO_APP
	movl	$1, %esi
	movq	%r14, %rdi
	call	*%rbx
#APP
# 52 "switch_x86_64_gcc.h" 1
	ldmxcsr 4(%rsp)
	fldcw 2(%rsp)
	
# 0 "" 2
#NO_APP
	movl	$40, %ecx
	movq	8(%rsp), %rsi
	xorq	%fs:(%rcx), %rsi
	jne	.L4
	addq	$24, %rsp
	.cfi_remember_state
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
.L4:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE0:
	.size	stackman_switch, .-stackman_switch
	.ident	"GCC: (Ubuntu 9.2.1-9ubuntu2) 9.2.1 20191008"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
