/* test including the switch definition directly into the main file,
 * rather than linking in an object file.  Only possible if the
 * implementation is inline asm.  Real assembler must be linked
 */
#undef STACKMAN_INLINE_ASM
#define STACKMAN_INLINE_ASM 1
#define STACKMAN_LINKAGE_STATIC
#include "stackman.h"
#ifndef STACKMAN_ASSEMBLY_SRC
#include "test.c"
#include "stackman_impl.h"
# else
#include <stdio.h>
int main(int argc, char* argv[])
{
	printf("no static with external asm\n");
	return 0;
}
#endif
