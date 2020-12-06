

; this code is adopted from the switch_aarch64_gcc.S
; which again was generated from switch_arm_gcc.h
; This code is also modelled on the hand-customized switch_arm_msvc.asm
; see https://docs.microsoft.com/en-us/cpp/build/arm64-windows-abi-conventions?view=msvc-160

	;ARM64

	EXPORT |stackman_switch|
	ALIGN
|stackman_switch| PROC
	; args "callback", "context" are in x0, x1
	; push non-volatile x18-x28 registers, in addition to fp,rl pair.
	; x29 is fp, x30 is lr.
	stp	x29, x30, [sp, #-176]!	; store fp,lr pair, allocate stack space
	mov	x29, sp					; and set fp pointing to old fp on stack
	; push non-volatile registers 18-x28 and fp regs d8-d15)
	; also  fpcr (fp control register)
	mrs    x2,fpcr
	stp	x2,  x18, [sp, #16]
	stp	x19, x20, [sp, #32]
	stp	x21, x22, [sp, #48]
	stp	x23, x24, [sp, #64]
	stp	x25, x26, [sp, #80]
	stp	x27, x28, [sp, #96]
	stp	d8,  d9,  [sp, #112]
	stp	d10, d11, [sp, #128]
	stp	d12, d13, [sp, #144]
	stp	d14, d15, [sp, #160]

	; args are x0=callback, x1=context
	; shuffle calling arguments into r0, r1 and r2
	; for the call to the callback.
	mov	x3, x0
	mov	x0, x1 ;context
	mov x2, sp ;stack pointer
	mov	x1, #0 ;operation
	mov	x19, x0 ;store these a bit in nv registers
	mov	x18, x3
	blr	x3     ;first callback
	mov	x2, x0 ;store new stackpointer in register, for next cb,
	; switch stack, maintaining fp offset
	sub x1, sp, x0
	mov sp, x0 ;switch stack pointer
	sub fp, fp, x1
	
	mov	x1, #1 ;operation
	mov	x0, x19 ;context
	blr	x18     ;second callback

	; restore registers from stack
	ldp	x2,  x18, [sp, #16]
	ldp	x19, x20, [sp, #32]
	ldp	x21, x22, [sp, #48]
	ldp	x23, x24, [sp, #64]
	ldp	x25, x26, [sp, #80]
	ldp	x27, x28, [sp, #96]
	ldp	d8,  d9,  [sp, #112]
	ldp	d10, d11, [sp, #128]
	ldp	d12, d13, [sp, #144]
	ldp	d14, d15, [sp, #160]
	msr    fpcr, x2
	;return
	ldp	x29, x30, [sp], #176
	ret
	ENDP

	EXPORT |stackman_call|
	ALIGN
|stackman_call| PROC
; args "callback", "context" are in x0, x1
	; push non-volatile x18-x28 registers, in addition to fp,rl pair.
	; x29 is fp, x30 is lr.
	stp	x29, x30, [sp, #-32]!	; store fp,lr pair, allocate stack space
	mov	x29, sp					; and set fp pointing to old fp on stack
	; push non-volatile register 18
	str	x18, [sp, #16]
	
	; args are x0 = callback, x1 = context, x2=stack
	mov	x3, x0
	mov	x0, x1 ;context
	mov	x1, x2 ;stack
	; store current sp in nv register
	mov x18, sp
	mov x2, sp ; new stack

	; change stack
	mov sp, x1
	mov	x1, #2 ; callback opcode
	blr	x3     ; call callback, with context, opcode, new stack

	; restore stack (could do: sub sp, fp #12)
	mov sp, x18

	; return
	ldr x18, [sp, #16]
	ldp	x29, x30, [sp], #32
	ret
	ENDP
	
	END
