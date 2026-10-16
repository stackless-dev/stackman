/* No supported ABI grows upward. This file compiles the ascending branch
 * of stackman_block_call. It does not link libstackman.
 */
#define STACKMAN_STACK_DIR 1
#include "stackman.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
	unsigned char raw[64 + STACKMAN_STACK_ALIGN * 2];
	uintptr_t aligned;
	unsigned char *base;
	void *call_sp;
	size_t extent;
	size_t size = 64;

	assert(STACKMAN_STACK_DIR == 1);

	aligned = ((uintptr_t)raw + STACKMAN_STACK_ALIGN - 1) &
		~(uintptr_t)(STACKMAN_STACK_ALIGN - 1);
	base = (unsigned char *)aligned;

	assert(stackman_block_call(base, size, &call_sp, &extent) == 1);
	assert(call_sp == (void *)base);
	assert(extent == size);
	assert((void *)STACKMAN_SP_ALIGN(call_sp) == call_sp);

	/* base+1 rounds up to the next alignment, and that gap is not extent. */
	assert(stackman_block_call(base + 1, size, &call_sp, &extent) == 1);
	assert(call_sp == (void *)(base + STACKMAN_STACK_ALIGN));
	assert(extent == size - (STACKMAN_STACK_ALIGN - 1));

	assert(stackman_block_call(base, 1, &call_sp, &extent) == 1);
	assert(call_sp == (void *)base && extent == 1);
	assert(stackman_block_call(base + 1, 1, &call_sp, &extent) == 0);
	assert(stackman_block_call(0, size, &call_sp, &extent) == 0);

	printf("test_block_asc ok\n");
	return 0;
}
