/* 
 * Implementation of the stack switching function, if it is done
 * in assembler. Gcc will invoke the C preprocesor on .S
 * files.
 */

#define STACKMAN_SWITCH_IMPL
#define STACKMAN_SWITCH_ASM 1
#include "platforms/platform.h"
