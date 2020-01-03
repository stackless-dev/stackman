/* testing of asm functionality on x86_64_gcc */

#define STACKMAN_SWITCH_ASM_TEST
#include "stackman_impl.h"

#ifdef STACKMAN_EXTERNAL_ASM
#include "test.c"
#else
int main(int *argc, char*argv[])
{
	return 0;
}
#endif