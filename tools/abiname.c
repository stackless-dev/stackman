/* output the stackman ABI name */
#include <stdio.h>
#include "stackman.h"

#define xstr(s) str(s)
#define str(s) #s

int main(int argc, char* argv[])
{
	printf("%s\n", xstr(STACKMAN_ABI));
	return 0;
}