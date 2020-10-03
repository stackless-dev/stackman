/* function to test the generation
 * of assembly code under e.g. Gcc.
 * use by calling, for example:
 *  cc -DSTACKMAN_SWITCH_IMPL -S -m32 -fcf-protection=none test.c
 * and examinine the generated test.s assembly code.  The fcf-protection
 * flag disables generation of intel CET compatible code, but stack switching
 * is not compatible with the proposed shadow stack.
 */

#define STACKMAN_VERBOSE
#include "platform.h"
