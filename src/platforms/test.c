/* function to test the generation
 * of assembly code under e.g. Gcc.
 * use by calling, for example:
 *  cc -DSTACKMAN_SWITCH_IMPL -S -m32  test.c
 * and examinine the generated test.s assembly code
 */

#define STACKMAN_VERBOSE
#include "platform.h"
