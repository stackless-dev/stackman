/*
 * test for the .S mechanism. two implementations are provided for x86_64_gcc, one
 * with pure assembly.  Enable this using the following:
 */

#define STACKMAN_PREFER_ASM 1
#include "stackman_impl.h"
