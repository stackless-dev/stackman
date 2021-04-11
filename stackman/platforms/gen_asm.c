/* function to test the generation
 * of assembly code under e.g. Gcc.
 * use by calling, for example:
 *  cc -S -m32 -fcf-protection=none gen_asm.c
 * and examinine the generated gen_asm.s assembly code.
 * -m32 selects 32 bit mode, use other directives to select a different platform.
 * The -fcf-protection flag disables generation of intel CET compatible code, but stack switching
 * is not compatible with the proposed shadow stack.  Only the latest compilers have it.
 */

#define STACKMAN_VERBOSE
#define STACKMAN_INLINE_ASM 1
#define STACKMAN_BUILD_LIB /* so that we don´t generate indirection shims */
#include "../stackman_impl.h"
