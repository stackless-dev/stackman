/* Paint tests for stackman_guard.c.
 * Built twice: once for this ABI's STACKMAN_STACK_DIR, and once with
 * STACKMAN_STACK_DIR=1. The ascending build does not call stackman_call.
 * No supported ABI grows that way. The call below runs only on the
 * descending build, which is every ABI this tree ships.
 */

#include "stackman_guard.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

enum { N = 32, GUARD = 8 };

static void
test_math(void)
{
	unsigned char raw[N];
	void *far;
	size_t i;
	unsigned char pattern = 0xa5;

#if STACKMAN_STACK_DIR == 0
	far = raw + N;
#else
	far = raw;
#endif

	raw[0] = 0x11;
	assert(stackman_guard_paint(0, 0, GUARD, pattern) == 0);
	assert(stackman_guard_check(0, 0, GUARD, pattern) == -1);
	assert(stackman_stack_unused(0, 0, pattern) == 0);
	assert(raw[0] == 0x11);

	assert(stackman_guard_paint(far, 4, 8, pattern) == 0);
	assert(stackman_guard_check(far, 4, 8, pattern) == -1);

	for (i = 0; i < N; i++)
		raw[i] = 0x11;
	assert(stackman_guard_paint(far, N, GUARD, pattern) == GUARD);
	for (i = 0; i < N; i++) {
		int in_guard;

#if STACKMAN_STACK_DIR == 0
		in_guard = i < GUARD;
#else
		in_guard = i >= N - GUARD;
#endif
		assert(raw[i] == (in_guard ? pattern : 0x11));
	}
	assert(stackman_guard_check(far, N, GUARD, pattern) == 1);

	/* The first byte farther than the guard is still in the block.
	 * Dirtying it leaves the guard intact. */
#if STACKMAN_STACK_DIR == 0
	raw[GUARD] = 0;
#else
	raw[N - GUARD - 1] = 0;
#endif
	assert(stackman_guard_check(far, N, GUARD, pattern) == 1);

#if STACKMAN_STACK_DIR == 0
	raw[0] = 0;
#else
	raw[N - 1] = 0;
#endif
	assert(stackman_guard_check(far, N, GUARD, pattern) == 0);

	assert(stackman_guard_paint(far, N, N, pattern) == N);
	assert(stackman_stack_unused(far, N, pattern) == N);
#if STACKMAN_STACK_DIR == 0
	raw[N - 1] = 0;
#else
	raw[0] = 0;
#endif
	assert(stackman_stack_unused(far, N, pattern) == N - 1);
}

#if STACKMAN_STACK_DIR == 0
static void *
call_cb(void *context, int opcode, void *old_sp)
{
	volatile unsigned char scratch[256];
	size_t i;

	(void)context;
	(void)opcode;
	(void)old_sp;
	for (i = 0; i < sizeof scratch; i++)
		scratch[i] = (unsigned char)i;
	return 0;
}

static void
test_call_block(void)
{
	unsigned char raw[4096 + 16];
	void *far;
	size_t extent;
	unsigned char pattern = 0x5a;

	assert(stackman_block_call(raw, sizeof raw, &far, &extent) == 1);
	assert(extent > 256 + 64);

	assert(stackman_guard_paint(far, extent, 64, pattern) == 64);
	assert(stackman_call(call_cb, 0, far) == 0);
	assert(stackman_guard_check(far, extent, 64, pattern) == 1);
}
#endif

int
main(void)
{
	test_math();
#if STACKMAN_STACK_DIR == 0
	test_call_block();
	printf("test_guard ok\n");
#else
	{
		unsigned char raw[128 + STACKMAN_STACK_ALIGN];
		uintptr_t aligned;
		unsigned char *base;
		void *far;
		size_t extent;

		aligned = ((uintptr_t)raw + STACKMAN_STACK_ALIGN - 1) &
			~(uintptr_t)(STACKMAN_STACK_ALIGN - 1);
		base = (unsigned char *)aligned;
		assert(stackman_block_call(base, 128, &far, &extent) == 1);
		assert(far == (void *)base);
		assert(extent == 128);
		assert(stackman_guard_paint(far, extent, GUARD, 0xa5) == GUARD);
		assert(stackman_guard_check(far, extent, GUARD, 0xa5) == 1);
	}
	printf("test_guard ascending ok\n");
#endif
	return 0;
}
