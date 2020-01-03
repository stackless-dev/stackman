/*
 * test for the .S mechanism. two implementations are provided for x86_64_gcc, one
 * with pure assembly.  Enable this using the following:
 */

#define STACKMAN_SWITCH_ASM_TEST
#define STACKMAN_SWITCH_ASM 1
#include "stackman_impl.h"
