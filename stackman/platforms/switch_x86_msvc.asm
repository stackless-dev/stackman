
.386
.model flat, c

.code

stackman_switch_raw PROC callback:DWORD, context:DWORD
  ;Assembler has already pushed the basepointer and saved it.
  ;save registers. EAX ECX and EDX are available for function use and thus
  ;do not have to be stored.
  push ebx
  push esi
  push edi
  
  mov esi, callback ;      /* save 'callback' for later */
  mov edi, context ;       /* save 'context' for later         */

  mov eax, esp

  push eax ;               /* arg 3: current (old) stack pointer */
  push 0;                  /* arg 2: opcode STACKMAN_OP_SAVE     */ 
  push edi ;               /* arg 1: context                     */
  call esi ;               /* call callback()                    */
  add esp, 12;
  
  mov ecx, eax;			   /* change stack pointer to eax, preserving */
  sub ecx, esp;            /* base pointer offset from esp */
  mov esp, eax;
  add ebp, ecx;
  

  push eax ;               /* arg 3: current (new) stack pointer */
  push 1;                  /* arg 2: opcode STACKMAN_OP_RESTORE  */ 
  push edi ;               /* arg 1: context                     */
  call esi ;               /* call callback()                    */
  add esp, 12

  pop  edi
  pop  esi
  pop  ebx
  ret
stackman_switch_raw ENDP


stackman_call PROC callback:DWORD, context:DWORD, stack_pointer:DWORD
  ;enter prolog has pushed ebp and saved esp in ebp

  mov eax, callback
  mov ecx, context
  mov edx, stack_pointer

  ; switch stack pointer
  test edx, edx
  je nullptr
  mov esp, stack_pointer
nullptr:
  push ebp  ;old stack pointer
  push 2	;STACKMAN_OP_CALL
  push ecx  ;context
  call eax  ;callback
  ret	; this assembles a LEAVE instruction which restores esp to ebp, then pops the ebp
stackman_call ENDP


end
