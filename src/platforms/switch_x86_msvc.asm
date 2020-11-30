
.386
.model flat, c

.code

stackman_switch_raw PROC callback:DWORD, context:DWORD
  
  ;save registers. EAX ECX and EDX are available for function use and thus
  ;do not have to be stored.
  push ebx
  push esi
  push edi
  push ebp
  
  mov esi, callback ;      /* save 'callback' for later */
  mov edi, context ;       /* save 'context' for later         */

  mov eax, esp

  push eax ;               /* arg 3: current (old) stack pointer */
  push 0;                  /* arg 2: opcode STACKMAN_OP_SAVE     */ 
  push edi ;               /* arg 1: context                     */
  call esi ;               /* call callback()                    */

  mov esp, eax;            /* change the stack pointer */

  push eax ;               /* arg 3: current (new) stack pointer */
  push 1;                  /* arg 2: opcode STACKMAN_OP_RESTORE  */ 
  push edi ;               /* arg 1: context                     */
  call esi ;               /* call callback()                    */
  add esp, 12

  pop  ebp
  pop  edi
  pop  esi
  pop  ebx
  ret
stackman_switch_raw ENDP

end
