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
/* Disable CFI directives on macOS - they have different semantics */
#define CFI_STARTPROC
#define CFI_ENDPROC
#define CFI_DEF_CFA_OFFSET(x)
#define CFI_OFFSET(r,o)
#define CFI_RESTORE(r)
#else
#define FUNCTION(name) .global name ; .type name, %function
#define LABEL(name) name:
#define TYPE_FUNCTION(name) .type name, %function
#define SIZE_FUNCTION(name) .size name, .-name
#define GNU_STACK .section .note.GNU-stack,"",@progbits
#define CFI_STARTPROC CFI_STARTPROC
#define CFI_ENDPROC CFI_ENDPROC
#define CFI_DEF_CFA_OFFSET(x) CFI_DEF_CFA_OFFSET()x
#define CFI_OFFSET(r,o) .cfi_offset r, o
#define CFI_RESTORE(r) CFI_RESTORE()r
#endif

	.arch armv8-a
	.file	"gen_asm.c"
	.text
	.align	2
	FUNCTION(stackman_switch)
LABEL(stackman_switch)
.LFB0:
	CFI_STARTPROC
	stp	x29, x30, [sp, -160]!
	CFI_DEF_CFA_OFFSET(160)
	CFI_OFFSET(29,-160)
	CFI_OFFSET(30,-152)
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
	CFI_OFFSET(19,-144)
	CFI_OFFSET(20,-136)
	CFI_OFFSET(21,-128)
	CFI_OFFSET(22,-120)
	CFI_OFFSET(23,-112)
	CFI_OFFSET(24,-104)
	CFI_OFFSET(25,-96)
	CFI_OFFSET(26,-88)
	CFI_OFFSET(27,-80)
	CFI_OFFSET(28,-72)
	CFI_OFFSET(72,-64)
	CFI_OFFSET(73,-56)
	CFI_OFFSET(74,-48)
	CFI_OFFSET(75,-40)
	CFI_OFFSET(76,-32)
	CFI_OFFSET(77,-24)
	CFI_OFFSET(78,-16)
	CFI_OFFSET(79,-8)
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
	CFI_RESTORE(30)
	CFI_RESTORE(29)
	CFI_RESTORE(78)
	CFI_RESTORE(79)
	CFI_RESTORE(76)
	CFI_RESTORE(77)
	CFI_RESTORE(74)
	CFI_RESTORE(75)
	CFI_RESTORE(72)
	CFI_RESTORE(73)
	CFI_RESTORE(27)
	CFI_RESTORE(28)
	CFI_RESTORE(25)
	CFI_RESTORE(26)
	CFI_RESTORE(23)
	CFI_RESTORE(24)
	CFI_RESTORE(21)
	CFI_RESTORE(22)
	CFI_RESTORE(19)
	CFI_RESTORE(20)
	CFI_DEF_CFA_OFFSET(0)
	ret
	CFI_ENDPROC
.LFE0:
	SIZE_FUNCTION(stackman_switch)
	.align	2
	FUNCTION(stackman_call)
LABEL(stackman_call)
.LFB1:
	CFI_STARTPROC
	stp	x29, x30, [sp, -32]!
	CFI_DEF_CFA_OFFSET(32)
	CFI_OFFSET(29,-32)
	CFI_OFFSET(30,-24)
	mov	x29, sp
	str	x19, [sp, 16]
	CFI_OFFSET(19,-16)
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
	CFI_RESTORE(30)
	CFI_RESTORE(29)
	CFI_RESTORE(19)
	CFI_DEF_CFA_OFFSET(0)
	ret
	CFI_ENDPROC
.LFE1:
	SIZE_FUNCTION(stackman_call)
	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
	GNU_STACK
