/* testing of inline asm functionality */

#define STACKMAN_INLINE_ASM 1
#include "stackman.h"
#include "stackman_impl.h"

#ifndef STACKMAN_EXTERNAL_ASM
#include "test.c"
#else
int main(int argc, char*argv[])
{
	return 0;
}
#endif