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
 * The use is a pre-allocated block passed to stackman_call.
 * stackman_block_call turns the malloc pair into `far` and `extent`.
 * `far` is the pointer passed as the new stack. `extent` is the byte
 * count toward near. For a range already held as two pointers,
 * extent = STACKMAN_SP_DIFF(far, near).
 *
 * Paint the near band, call, then check it. Paint the whole extent
 * and stackman_stack_unused reports how much of that block the call
 * left untouched, so the next allocation can be sized from a real run.
 *
 * A stackman_switch snapshot is the other shape. It is the used range
 * between the saved near pointer and far. There is no unused tail
 * inside it. The main stack's far is often STACKMAN_SP_FURTHEST,
 * which is not a paintable address. Pass extent 0 when the near bound
 * is unknown. The functions then do not read or write memory.
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
 * Same coordinates as paint: far, then extent toward near.
 * extent 0 returns 0.
 */
size_t stackman_stack_unused(void *far, size_t extent, unsigned char pattern);

#endif
