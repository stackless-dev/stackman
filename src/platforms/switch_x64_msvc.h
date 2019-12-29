/* The actual stack saving function, which just stores the stack,
 * this declared in an .asm file
 */
#ifdef STACKMAN_SWITCH_IMPL
#define STACKMAN_EXTERNAL_ASM switch_x64_msvc.asm
#endif
