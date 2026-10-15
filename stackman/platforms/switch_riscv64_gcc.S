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
 * - FP regs: fs0-fs11 (callee-saved floating point)
 */
	FUNCTION(stackman_switch)
LABEL(stackman_switch)
.LFB0:
	CFI_STARTPROC

	/* 208 bytes, 16-byte aligned.
	 * ra, s0-s11, and fs0-fs11 occupy bytes 8 through 207.
	 * The pad at byte 0 holds fcsr, so the pointer passed to the
	 * callback includes it. A thread switch saves the whole register
	 * (rounding mode and accrued exception flags). stackman_call does
	 * not touch fcsr.
	 */
	addi	sp, sp, -208
	CFI_DEF_CFA_OFFSET(208)
	
	/* Save return address and callee-saved registers */
	sd	ra, 200(sp)
	CFI_OFFSET(1, -8)	/* ra is x1 */
	sd	s0, 192(sp)
	CFI_OFFSET(8, -16)
	sd	s1, 184(sp)
	CFI_OFFSET(9, -24)
	sd	s2, 176(sp)
	CFI_OFFSET(18, -32)
	sd	s3, 168(sp)
	CFI_OFFSET(19, -40)
	sd	s4, 160(sp)
	CFI_OFFSET(20, -48)
	sd	s5, 152(sp)
	CFI_OFFSET(21, -56)
	sd	s6, 144(sp)
	CFI_OFFSET(22, -64)
	sd	s7, 136(sp)
	CFI_OFFSET(23, -72)
	sd	s8, 128(sp)
	CFI_OFFSET(24, -80)
	sd	s9, 120(sp)
	CFI_OFFSET(25, -88)
	sd	s10, 112(sp)
	CFI_OFFSET(26, -96)
	sd	s11, 104(sp)
	CFI_OFFSET(27, -104)
	
	/* Save callee-saved floating point registers */
	fsd	fs0, 96(sp)
	fsd	fs1, 88(sp)
	fsd	fs2, 80(sp)
	fsd	fs3, 72(sp)
	fsd	fs4, 64(sp)
	fsd	fs5, 56(sp)
	fsd	fs6, 48(sp)
	fsd	fs7, 40(sp)
	fsd	fs8, 32(sp)
	fsd	fs9, 24(sp)
	fsd	fs10, 16(sp)
	fsd	fs11, 8(sp)

	/* frcsr after the first FP spill, so a lazy FP enable has run.
	 * t0 is free. The eight-byte pad keeps the 16-byte alignment.
	 */
	frcsr	t0
	sd	t0, 0(sp)
	
	/* Preserve callback and context in callee-saved registers */
	mv	s2, a0		/* s2 = callback */
	mv	s3, a1		/* s3 = context */
	
	/* First callback: callback(context, opcode, sp) */
	mv	a0, s3		/* a0 = context */
	li	a1, 0		/* a1 = STACKMAN_OP_SAVE */
	mv	a2, sp		/* a2 = current sp */
	jalr	s2		/* call callback */
	
	/* Save returned pointer and switch stack */
	mv	s4, a0		/* s4 = returned value */
	mv	sp, a0		/* switch to new stack */
	
	/* Second callback: callback(context, opcode, sp) */
	mv	a0, s3		/* a0 = context */
	li	a1, 1		/* a1 = STACKMAN_OP_RESTORE */
	mv	a2, sp		/* a2 = new sp */
	jalr	s2		/* call callback */

	/* Reload fcsr from the stack now current, before the epilogue
	 * pops fs0-fs11. a0 is the callback result and stays there.
	 */
	ld	t0, 0(sp)
	fscsr	t0
	
	/* Restore callee-saved floating point registers */
	fld	fs11, 8(sp)
	fld	fs10, 16(sp)
	fld	fs9, 24(sp)
	fld	fs8, 32(sp)
	fld	fs7, 40(sp)
	fld	fs6, 48(sp)
	fld	fs5, 56(sp)
	fld	fs4, 64(sp)
	fld	fs3, 72(sp)
	fld	fs2, 80(sp)
	fld	fs1, 88(sp)
	fld	fs0, 96(sp)
	
	/* Restore callee-saved integer registers */
	ld	s11, 104(sp)
	CFI_RESTORE(27)
	ld	s10, 112(sp)
	CFI_RESTORE(26)
	ld	s9, 120(sp)
	CFI_RESTORE(25)
	ld	s8, 128(sp)
	CFI_RESTORE(24)
	ld	s7, 136(sp)
	CFI_RESTORE(23)
	ld	s6, 144(sp)
	CFI_RESTORE(22)
	ld	s5, 152(sp)
	CFI_RESTORE(21)
	ld	s4, 160(sp)
	CFI_RESTORE(20)
	ld	s3, 168(sp)
	CFI_RESTORE(19)
	ld	s2, 176(sp)
	CFI_RESTORE(18)
	ld	s1, 184(sp)
	CFI_RESTORE(9)
	ld	s0, 192(sp)
	CFI_RESTORE(8)
	ld	ra, 200(sp)
	CFI_RESTORE(1)
	
	/* Deallocate frame and return */
	addi	sp, sp, 208
	CFI_DEF_CFA_OFFSET(0)
	ret

	CFI_ENDPROC
.LFE0:
	SIZE_FUNCTION(stackman_switch)

/*
 * void *stackman_call(stackman_cb_t callback, void *context, void *new_stack)
 *
 * Calls callback(context, STACKMAN_OP_CALL, old_sp). A non-zero new_stack
 * replaces sp for that call. Zero leaves the current stack in place.
 * The third argument is the stack pointer from before the switch.
 *
 * Register usage:
 * - a0: callback function pointer, then context
 * - a1: context pointer, then opcode
 * - a2: requested stack pointer, then the previous stack pointer
 * - s0: previous stack pointer (callee-saved)
 * - t0: callback
 * - t1: requested stack pointer
 */
	.align	1
	FUNCTION(stackman_call)
LABEL(stackman_call)
.LFB1:
	CFI_STARTPROC

	/* Allocate frame for ra and s0 (callee-saved) */
	addi	sp, sp, -16
	CFI_DEF_CFA_OFFSET(16)
	
	/* Save return address and s0 (we'll use it to preserve old sp) */
	sd	ra, 8(sp)
	CFI_OFFSET(1, -8)
	sd	s0, 0(sp)
	CFI_OFFSET(8, -16)
	
	/* Save old stack pointer in s0 (callee-saved register) */
	mv	s0, sp
	
	/* Preserve the callback and the requested stack. Both are
	 * caller-saved, and both are consumed before jalr. */
	mv	t0, a0			/* t0 = callback */
	mv	t1, a2			/* t1 = requested stack, or zero */
	
	/* callback(context, STACKMAN_OP_CALL, previous sp) */
	mv	a0, a1			/* a0 = context */
	li	a1, 2			/* a1 = STACKMAN_OP_CALL */
	mv	a2, s0			/* a2 = previous stack pointer */
	
	/* A null stack means "keep the current one". */
	beqz	t1, .Lcall_keep_sp
	mv	sp, t1
.Lcall_keep_sp:
	jalr	t0			/* call callback */
	
	/* Restore old stack pointer from s0 */
	mv	sp, s0
	
	/* Restore s0 and return address */
	ld	s0, 0(sp)
	CFI_RESTORE(8)
	ld	ra, 8(sp)
	CFI_RESTORE(1)
	
	addi	sp, sp, 16
	CFI_DEF_CFA_OFFSET(0)
	ret

	CFI_ENDPROC
.LFE1:
	SIZE_FUNCTION(stackman_call)

/* Mark stack as non-executable */
GNU_STACK
