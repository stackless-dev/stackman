/*
 * this file is provided for where inline assembly is not
 * dependable, such as with clang on linux x86_64.
 * The file is generated using
 * "gcc -DSTACKMAN_SWITCH_IMPL -S test.c" and then copying the code
 * from test.s into this file.
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
	movq	%rdi, %rax
	movq	%rsi, %rcx
#APP
# 49 "switch_x86_64_gcc.h" 1
	fstcw 10(%rsp)
	stmxcsr 12(%rsp)
	
# 0 "" 2
# 55 "switch_x86_64_gcc.h" 1
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
# 59 "switch_x86_64_gcc.h" 1
	movq %rax, %rsp
# 0 "" 2
#NO_APP
	movl	$1, %esi
	movq	%r14, %rdi
	call	*%rbx
#APP
# 63 "switch_x86_64_gcc.h" 1
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
