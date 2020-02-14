/*
 * this file is provided for where inline assembly is not
 * dependable, such as with clang on linux x86_64 or if
 * STACKNAN_PREFER_ASM is defined.
 * The file is generated using
 * "arm-linux-gnueabi-gcc -DSTACKMAN_SWITCH_IMPL -S test.c" and then copying the code
 * from test.s into this file.
 * 
 */
	.arch armv5t
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 34, 0
	.eabi_attribute 18, 4
	.file	"test.c"
	.text
	.align	2
	.global	stackman_switch
	.syntax unified
	.arm
	.fpu softvfp
	.type	stackman_switch, %function
stackman_switch:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r3, r0
	mov	r0, r1
	.syntax divided
@ 78 "switch_arm_gcc.h" 1
	mov r2, sp
@ 0 "" 2
	.arm
	.syntax unified
	mov	r1, #0
	mov	r5, r0
	mov	r4, r3
	blx	r3
	mov	r2, r0
	.syntax divided
@ 82 "switch_arm_gcc.h" 1
	mov sp, r0
@ 0 "" 2
	.arm
	.syntax unified
	mov	r1, #1
	mov	r0, r5
	blx	r4
	pop	{r3, r4, r5, r6, r7, r8, r9, r10, fp, pc}
	.size	stackman_switch, .-stackman_switch
	.ident	"GCC: (Ubuntu 9.2.1-9ubuntu2) 9.2.1 20191008"
	.section	.note.GNU-stack,"",%progbits
