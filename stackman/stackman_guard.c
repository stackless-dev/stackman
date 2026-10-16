/* Optional stack paint. See stackman_guard.h. No library calls. */

#include "stackman_guard.h"

/* from_near 0 is the byte nearest the overflow end. */
static unsigned char *
byte_at(void *far, size_t extent, size_t from_near)
{
#if STACKMAN_STACK_DIR == 0
	return (unsigned char *)far - extent + from_near;
#else
	return (unsigned char *)far + extent - 1 - from_near;
#endif
}

static int
guard_described(size_t extent, size_t guard_bytes)
{
	return extent != 0 && guard_bytes != 0 && guard_bytes <= extent;
}

size_t
stackman_guard_paint(void *far, size_t extent, size_t guard_bytes,
		     unsigned char pattern)
{
	size_t i;

	if (!guard_described(extent, guard_bytes))
		return 0;
	for (i = 0; i < guard_bytes; i++)
		*byte_at(far, extent, i) = pattern;
	return guard_bytes;
}

int
stackman_guard_check(void *far, size_t extent, size_t guard_bytes,
		     unsigned char pattern)
{
	size_t i;

	if (!guard_described(extent, guard_bytes))
		return -1;
	for (i = 0; i < guard_bytes; i++) {
		if (*byte_at(far, extent, i) != pattern)
			return 0;
	}
	return 1;
}

size_t
stackman_stack_unused(void *far, size_t extent, unsigned char pattern)
{
	size_t i;

	for (i = 0; i < extent; i++) {
		if (*byte_at(far, extent, i) != pattern)
			return i;
	}
	return extent;
}
