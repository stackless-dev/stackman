/* The actual stack saving function, which just stores the stack,
 * this declared in an .asm file
 */
#ifndef STACKMAN_ASSEMBLY_SRC
#define STACKMAN_ASSEMBLY_SRC switch_x64_msvc.asm
#define STACKMAN_HAVE_CALL 1
#define STACKMAN_STACK_ALIGN 16
#endif
