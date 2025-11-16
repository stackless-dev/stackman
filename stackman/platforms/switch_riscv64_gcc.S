/*
 * RISC-V RV64 (LP64D ABI) stack switching implementation
 * 
 * This file provides stack manipulation for RISC-V 64-bit (RV64) using the
 * LP64D ABI (64-bit pointers, hardware double-precision floating point).
 * 
 * RISC-V RV64 LP64D Calling Convention:
 * - Arguments: a0-a7 (x10-x17)
 * - Return: a0-a1 (x10-x11)
 * - Callee-saved: s0-s11 (x8-x9, x18-x27), sp (x2)
 * - Caller-saved: a0-a7, t0-t6, ra (x1)
 * - Stack pointer: sp (x2) - must be 16-byte aligned
 * - Frame pointer: s0/fp (x8) - optional
 */

/* ELF metadata and CFI directives */
#define FUNCTION(name) .globl name ; .type name, @function
#define LABEL(name) name:
#define TYPE_FUNCTION(name) .type name, @function
#define SIZE_FUNCTION(name) .size name, .-name
#define GNU_STACK .section .note.GNU-stack,"",@progbits
#define CFI_STARTPROC .cfi_startproc
#define CFI_ENDPROC .cfi_endproc
#define CFI_DEF_CFA_OFFSET(x) .cfi_def_cfa_offset x
#define CFI_OFFSET(r,o) .cfi_offset r, o
#define CFI_RESTORE(r) .cfi_restore r

	.file	"switch_riscv64_gcc.S"
	.text
	.align	1

/* 
 * void *stackman_switch(stackman_cb_t callback, void *context)
 * 
 * Saves current stack state, calls callback twice with stack pointer,
 * and switches to new stack returned by first callback invocation.
 * 
 * Register usage:
 * - a0: callback function pointer (first arg)
 * - a1: context pointer (second arg)
 * - Callee-saved regs: s0-s11 must be preserved
 */
	FUNCTION(stackman_switch)
LABEL(stackman_switch)
.LFB0:
	CFI_STARTPROC

	/* TODO: Calculate frame size based on saved registers
	 * Need to save:
	 * - ra (return address)
	 * - s0-s11 (callee-saved registers)
	 * - fp/s0 for frame pointer
	 * 
	 * Total: ~14 registers * 8 bytes = 112 bytes (round to 128 for alignment)
	 */

	/* TODO: Allocate stack frame and save registers */
	/* addi sp, sp, -128 */
	/* sd ra, 120(sp) */
	/* sd s0, 112(sp) */
	/* sd s1-s11, ... */

	/* TODO: Set up frame pointer */
	/* addi s0, sp, 128 */

	/* TODO: Call callback with current sp */
	/* First call: callback(context, sp) */
	/* mv a1, sp          # second arg = current sp */
	/* mv a2, a0          # save callback for later */
	/* mv a0, a1_original # first arg = context */
	/* jalr a2            # call callback */

	/* TODO: Switch to new stack returned in a0 */
	/* mv sp, a0 */

	/* TODO: Second callback invocation with new sp */
	/* Same pattern as first call */

	/* TODO: Restore registers from new stack */
	/* ld s11, offset(sp) */
	/* ... */
	/* ld ra, offset(sp) */

	/* TODO: Deallocate frame and return */
	/* addi sp, sp, 128 */
	/* ret */

	/* PLACEHOLDER - just return for now */
	ret

	CFI_ENDPROC
.LFE0:
	SIZE_FUNCTION(stackman_switch)

/* 
 * void *stackman_call(stackman_cb_t callback, void *context, void *new_stack)
 * 
 * Similar to stackman_switch but uses explicitly provided stack pointer.
 * 
 * Register usage:
 * - a0: callback function pointer
 * - a1: context pointer
 * - a2: new stack pointer
 */
	.align	1
	FUNCTION(stackman_call)
LABEL(stackman_call)
.LFB1:
	CFI_STARTPROC

	/* TODO: Similar to stackman_switch but simpler
	 * - Save minimal state on current stack
	 * - Switch to new_stack (a2)
	 * - Call callback
	 * - Return
	 */

	/* PLACEHOLDER */
	ret

	CFI_ENDPROC
.LFE1:
	SIZE_FUNCTION(stackman_call)

/* Mark stack as non-executable */
GNU_STACK
