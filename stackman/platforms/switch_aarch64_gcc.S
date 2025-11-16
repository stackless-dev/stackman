/*
 * this file is provided for where inline assembly is not
 * dependable, such as with clang or if
 * STACKMAN_PREFER_ASM is defined. It provides a stable
 * implementation that is independent of subtleties of inline
 * assembly code which may change with compiler versions.
 * The file is generated using
 * "aarch64-linux-gnu-gcc -S gen_asm.c"
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
#else
#define FUNCTION(name) .global name ; .type name, %function
#define LABEL(name) name:
#define TYPE_FUNCTION(name) .type name, %function
#define SIZE_FUNCTION(name) .size name, .-name
#define GNU_STACK .section .note.GNU-stack,"",@progbits
#endif

	.arch armv8-a
	.file	"gen_asm.c"
	.text
	.align	2
	FUNCTION(stackman_switch)
LABEL(stackman_switch)
.LFB0:
	.cfi_startproc
	stp	x29, x30, [sp, -160]!
	.cfi_def_cfa_offset 160
	.cfi_offset 29, -160
	.cfi_offset 30, -152
	mov	x29, sp
	stp	x19, x20, [sp, 16]
	stp	x21, x22, [sp, 32]
	stp	x23, x24, [sp, 48]
	stp	x25, x26, [sp, 64]
	stp	x27, x28, [sp, 80]
	stp	d8, d9, [sp, 96]
	stp	d10, d11, [sp, 112]
	stp	d12, d13, [sp, 128]
	stp	d14, d15, [sp, 144]
	.cfi_offset 19, -144
	.cfi_offset 20, -136
	.cfi_offset 21, -128
	.cfi_offset 22, -120
	.cfi_offset 23, -112
	.cfi_offset 24, -104
	.cfi_offset 25, -96
	.cfi_offset 26, -88
	.cfi_offset 27, -80
	.cfi_offset 28, -72
	.cfi_offset 72, -64
	.cfi_offset 73, -56
	.cfi_offset 74, -48
	.cfi_offset 75, -40
	.cfi_offset 76, -32
	.cfi_offset 77, -24
	.cfi_offset 78, -16
	.cfi_offset 79, -8
	mov	x3, x0
	mov	x0, x1
#APP
// 59 "../platforms/switch_aarch64_gcc.h" 1
	mov x2, sp
// 0 "" 2
#NO_APP
	mov	w1, 0
	mov	x20, x0
	mov	x19, x3
	blr	x3
	mov	x2, x0
#APP
// 63 "../platforms/switch_aarch64_gcc.h" 1
	mov sp, x0
// 0 "" 2
#NO_APP
	mov	w1, 1
	mov	x0, x20
	blr	x19
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	d8, d9, [sp, 96]
	ldp	d10, d11, [sp, 112]
	ldp	d12, d13, [sp, 128]
	ldp	d14, d15, [sp, 144]
	ldp	x29, x30, [sp], 160
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 78
	.cfi_restore 79
	.cfi_restore 76
	.cfi_restore 77
	.cfi_restore 74
	.cfi_restore 75
	.cfi_restore 72
	.cfi_restore 73
	.cfi_restore 27
	.cfi_restore 28
	.cfi_restore 25
	.cfi_restore 26
	.cfi_restore 23
	.cfi_restore 24
	.cfi_restore 21
	.cfi_restore 22
	.cfi_restore 19
	.cfi_restore 20
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE0:
	SIZE_FUNCTION(stackman_switch)
	.align	2
	FUNCTION(stackman_call)
LABEL(stackman_call)
.LFB1:
	.cfi_startproc
	stp	x29, x30, [sp, -32]!
	.cfi_def_cfa_offset 32
	.cfi_offset 29, -32
	.cfi_offset 30, -24
	mov	x29, sp
	str	x19, [sp, 16]
	.cfi_offset 19, -16
	mov	x4, x0
	mov	x0, x1
	mov	x3, x2
#APP
// 78 "../platforms/switch_aarch64_gcc.h" 1
	mov x19, sp
// 0 "" 2
// 79 "../platforms/switch_aarch64_gcc.h" 1
	mov x2, sp
// 0 "" 2
#NO_APP
	cbz	x3, .L4
#APP
// 84 "../platforms/switch_aarch64_gcc.h" 1
	mov sp, x3
// 0 "" 2
#NO_APP
.L4:
	mov	w1, 2
	blr	x4
#APP
// 88 "../platforms/switch_aarch64_gcc.h" 1
	mov sp, x19
// 0 "" 2
#NO_APP
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	.cfi_restore 30
	.cfi_restore 29
	.cfi_restore 19
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE1:
	SIZE_FUNCTION(stackman_call)
	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
	GNU_STACK
