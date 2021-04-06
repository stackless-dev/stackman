#include "..\..\stackman\stackman.h"

/*
 * template file to create assembly code (template.asm) to modify and add to real assembler.
 */

void* stackman_call_templ(stackman_cb_t callback, void* context, void* stack)
{
    // We use this variabl here for the template generation.  Int the modified assembly
    // code, we will store the ebp (base pointer) in that place on the stack, 
    // before storing the original unmodified stack pointer there.
    void* localvar = stack;
    return callback(context, 2, localvar);
 
}