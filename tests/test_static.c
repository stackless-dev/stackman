/* test including the swithc definition directly into the main file,
 * rather than linking in an object file.  Only possible if the
 * implementation is inline asm.  Real assembler must be linked
 */
#define STACKMAN_LINKAGE_STATIC
#include "stackman.h"
#ifndef STACKMAN_EXTERNAL_ASM
#include "test.c"
#define STACKMAN_SWITCH_IMPL
#include "platforms/platform.h"
# else
#include <stdio.h>
int main(int argc, char* argv[])
{
	printf("no static with external asm\n");
	return 0;
}
#endif