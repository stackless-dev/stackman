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
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$8, %rsp
	.cfi_offset 15, -24
	.cfi_offset 14, -32
	.cfi_offset 13, -40
	.cfi_offset 12, -48
	.cfi_offset 3, -56
	movq	%rdi, %rax
	movq	%rsi, %rcx
#APP
# 29 "switch_x86_64_gcc.h" 1
	movq %rsp, %rdx
# 0 "" 2
#NO_APP
	movl	$0, %esi
	movq	%rcx, %r14
	movq	%rcx, %rdi
	movq	%rax, %rbx
	call	*%rax
	movq	%rax, %rdx
#APP
# 33 "switch_x86_64_gcc.h" 1
	movq %rax, %rsp
# 0 "" 2
#NO_APP
	movl	$1, %esi
	movq	%r14, %rdi
	call	*%rbx
	addq	$8, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
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
