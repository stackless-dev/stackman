/*
 * this file is provided for where inline assembly is not
 * dependable, such as with clang on linux x86_64 or if
 * STACKMAN_INLINE_ASM is false
 * The file is generated using
 * "gcc -DSTACKMAN_SWITCH_IMPL -S -m32 test.c" and then copying the code
 * from test.s into this file.
 * 
 */
	.file	"test.c"
	.text
	.globl	stackman_switch
	.type	stackman_switch, @function
stackman_switch:
.LFB0:
	.cfi_startproc
	endbr32
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
# 44 "switch_x86_gcc.h" 1
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
	.ident	"GCC: (Ubuntu 9.2.1-9ubuntu2) 9.2.1 20191008"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 4
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 4
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 4
4:
