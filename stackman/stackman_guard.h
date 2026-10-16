#ifndef STACKMAN_GUARD_H
#define STACKMAN_GUARD_H

/*
 * Optional paint of a caller-owned stack block. Not part of the switch.
 * Compile stackman_guard.c into the program that wants it. The switch
 * library does not include this file.
 *
 * No library calls. The bytes are ordinary stores. Nothing is registered
 * with the operating system, and nothing programs a hardware limit.
 *
 * `far` is the far boundary, in the same sense as the pointer passed to
 * stackman_call: the initial stack pointer. The block lies strictly
 * nearer than `far`. `extent` is how many bytes that is.
 *
 * The near bound is the overflow side. It is often unknown. Pass
 * extent 0 in that case. The functions then do not read or write memory.
 * stackman_call usually targets a block the caller allocated, and then
 * both ends are known:
 *
 *   extent = STACKMAN_SP_DIFF(far, near);
 *
 * The direction comes from STACKMAN_STACK_DIR. On a descending stack the
 * bytes are [far - extent, far). On an ascending stack they are
 * [far, far + extent). The guard is the `guard_bytes` at the near end,
 * the end a growing stack reaches last.
 *
 * A check only sees what was overwritten before the call. It does not
 * stop the write. stackman_stack_unused is meaningful after the whole
 * extent was painted with `pattern` and the stack then grew from `far`
 * toward near, leaving a solid run of `pattern` at the near end. A
 * pattern byte written by the program counts as untouched.
 */

#include "stackman.h"
#include <stddef.h>

size_t stackman_guard_paint(void *far, size_t extent, size_t guard_bytes,
			    unsigned char pattern);

/* 1: every guard byte still holds pattern.
 * 0: a guard byte differs.
 * -1: there is no guard (extent is 0, guard_bytes is 0, or
 *     guard_bytes is larger than extent).
 */
int stackman_guard_check(void *far, size_t extent, size_t guard_bytes,
			 unsigned char pattern);

/* How many bytes from the near end toward far still hold pattern.
 * extent 0 returns 0.
 */
size_t stackman_stack_unused(void *far, size_t extent, unsigned char pattern);

#endif
