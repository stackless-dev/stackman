

; this code is adopted from the switch_arm_gcc.S
; which again was generated from switch_arm_gcc.h

	THUMB

	EXPORT |stackman_switch|
	ALIGN
|stackman_switch| PROC
	push	{r4 - r10, fp, lr}  ;9 x 4bytes
	add fp, sp, #28 ; (4x7) fp must point to the previous fp on the stack
	
	; also push floating point registers and fpscr (status&control register)
	vpush	{q4-q7}
	vmrs    r2,fpscr
	push	{r2}				;5 x 4bytes
	; total of 14 words pushed, satisfying 8 byte stack alignment.

	; args are r0=callback, r1=context
	; shuffle calling arguments into r0, r1 and r2
	; for the call to the callback.
	mov	r3, r0
	mov	r0, r1 ;context
	mov r2, sp ;stack pointer
	mov	r1, #0 ;operation
	mov	r5, r0 ;store these a bit
	mov	r4, r3
	blx	r3     ;first callback
	mov	r2, r0 ;store new stackpointer in register, for next cb,
	; switch stack pointer, maintaing fp offset
	sub r1, sp, r0
	mov sp, r0 ;switch stack pointer
	sub fp, fp, r1
	
	mov	r1, #1 ;operation
	mov	r0, r5 ;context
	blx	r4     ;second callback
	; pop and return
	pop {r2}
	vmsr    fpscr, r2
	vpop	{q4, q5, q6, q7}
	pop	{r4-r9, r10, fp, pc}
	ENDP

	EXPORT |stackman_call|
	ALIGN
|stackman_call| PROC
	; args are r0 = callback, r1 = context, r2=stack
	push	{r4, fp, lr}
	add	fp, sp, #4  ;fp must always point to the last fp (r11)
	mov	r3, r0
	mov	r0, r1 ;context
	mov	r1, r2 ;stack
	; store current sp in nv register
	mov r4, sp
	mov r2, sp ; new stack

	; change stack
	mov sp, r1
	mov	r1, #2 ; callback opcode
	blx	r3     ; call callback, with context, opcode, new stack
	; restore stack (could do: sub sp, fp #12)
	mov sp, r4
	; return
	pop	{r4, fp, pc}
	ENDP

	END
