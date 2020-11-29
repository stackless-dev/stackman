/*
 * this file is provided for where inline assembly is not
 * dependable, such as with clang on linux x86 or if
 * STACKMAN_INLINE_ASM is false. It provides a stable
 * implementation that is independent of subtleties of inline
 * assembly code which may change with compiler versions.
 * The file is generated using
 * "gcc -DSTACKMAN_SWITCH_IMPL -S -m32 -fcf-protection=none test.c"
 * and then copying the code from test.s into this file.
 * 
 */
	.file	"test.c"
	.text
	.globl	stackman_switch
	.type	stackman_switch, @function
stackman_switch:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	.cfi_offset 7, -12
	.cfi_offset 6, -16
	.cfi_offset 3, -20
	movl	8(%ebp), %eax
	movl	12(%ebp), %edx
#APP
# 48 "switch_x86_gcc.h" 1
	subl $12, %esp
movl %eax, %esi
movl %edx, %edi
movl %esp, 8(%esp)
movl $0, 4(%esp)
movl %edi, 0(%esp)
call *%esi
movl %eax, %esp
movl %eax, 8(%esp)
movl $1, 4(%esp)
movl %edi, 0(%esp)
call *%esi
movl %eax, %eax
addl $12, %esp

# 0 "" 2
#NO_APP
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%edi
	.cfi_restore 7
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	stackman_switch, .-stackman_switch
	.globl	stackman_call
	.type	stackman_call, @function
stackman_call:
.LFB1:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$8, %esp
	.cfi_offset 3, -12
#APP
# 96 "switch_x86_gcc.h" 1
	movl %esp, %ebx
# 0 "" 2
# 97 "switch_x86_gcc.h" 1
	movl %esp, %eax
# 0 "" 2
#NO_APP
	movl	16(%ebp), %edx
#APP
# 100 "switch_x86_gcc.h" 1
	movl %edx, %esp
# 0 "" 2
#NO_APP
	pushl	%eax
	pushl	$2
	pushl	12(%ebp)
	call	*8(%ebp)
#APP
# 104 "switch_x86_gcc.h" 1
	movl %ebx, %esp
# 0 "" 2
#NO_APP
	movl	-4(%ebp), %ebx
	leave
	.cfi_restore 5
	.cfi_restore 3
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE1:
	.size	stackman_call, .-stackman_call
	.ident	"GCC: (Ubuntu 9.3.0-10ubuntu2) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
