#include "stackman.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



/* simple test ithout modifying stack pointers.
 * test that the callback is called wiht valid
 * stackpoiners and the stage member in the 
 * correct order.
 */
typedef struct ctxt01
{
	void *sp[2];
} ctxt01;

/* test retrieval of stack pointer */
void *test_01_cb(void* context, int _opcode, void *sp)
{
	ctxt01 *c = (ctxt01*)context;
	stackman_op_t opcode = (stackman_op_t)_opcode;
	assert(opcode == STACKMAN_OP_SAVE || opcode == STACKMAN_OP_RESTORE);

	/* check that stages are init in order */
	assert(!c->sp[1]);
	if (opcode == STACKMAN_OP_RESTORE)
		assert(c->sp[0]);
	else
		assert(!c->sp[1]);
	c->sp[opcode] = sp;
	if (opcode == (int)STACKMAN_OP_SAVE)
		return sp ;/* no stack switching */
	return (void*)1; /* test return argument */
}
void test_01(void)
{
	/* since we are not switching stacks, it is ok to keep context on stack */
	ctxt01 c;
	void *sp;
	memset(&c, 0, sizeof(c));

	sp = stackman_switch(&test_01_cb, &c);

	assert(sp == (void*)1);
	assert(c.sp[0] == c.sp[1]);
	assert(STACKMAN_SP_LE(c.sp[0], (void*)&c));
}


/* a more complex test, performing a long jump out of a subroutine
 * the context now stores a stub that can be jumped to 
 */
typedef struct ctxt02
{
	void *stack_far, *stack_near;
	void *buf;
	size_t size;
	/* every call to cb logs val in log and increments */
	int val;
	int log[10];
	int counter;
} ctxt02;

/* helpers to save and restore stack contents to a buffer */
void save_stack(void *sp, void *buf, size_t size)
{
	if (STACKMAN_DIRECTION == 0)
		memcpy(buf, sp, size);
	else
		memcpy(buf, (void*)((char*)sp-size), size);
}

void restore_stack(void *sp, void *buf, size_t size)
{
	if (STACKMAN_DIRECTION == 0)
		memcpy(sp, buf, size);
	else
		memcpy((void*)((char*)sp-size), buf, size);
}

void *test_02_cb(void* context, int opcode, void *sp)
{
	ctxt02 *c = (ctxt02*)context;
	c->log[c->counter++] = c->val;
	if (c->val == 0) {
		/* storing stub */
		if (opcode == (int)STACKMAN_OP_SAVE) {
			c->stack_near = sp;
			c->size = STACKMAN_SP_DIFF((char*)c->stack_far, (char*)c->stack_near);
			c->buf = malloc(c->size);
			save_stack(sp, c->buf, c->size);
		}
		return sp;
	}
	/* now we are jumping.  no need to save old stack */
	if (opcode == (int)STACKMAN_OP_SAVE) {
		return c->stack_near;
	} else {
		restore_stack(c->stack_near, c->buf, c->size);
		return sp;
	}
}
 
void test_02_called(ctxt02 *c)
{
	/* right, perform the jump to the original stack pointer */
	c->val = 1;
	stackman_switch(&test_02_cb, c);
}
void test_02(void)
{
	int stack_marker;
	static ctxt02 c;  /* keep away from stack */
	memset(&c, 0, sizeof(c));

	/* far end of stack, add buffer to catch memory backed registers, etc. */
	c.stack_far = STACKMAN_SP_ADD((char*)&stack_marker, 32);

	/* first time around, get the stack pointer */
	stackman_switch(&test_02_cb, &c);

	if (c.val == 0) {
		assert(c.counter == 2);
		/* first time.  We now have the sp to return to the original one
		 * lets long jump out of a recursive function here
		 */
		assert(c.buf);
		assert(c.size);
		test_02_called(&c);
		assert(0); /* never reach this */
	}
	assert(c.val == 1);
	assert(c.counter == 4);
}

int main(int argc, char*argv[])
{
	test_01();
	printf("test_01 ok\n");
	test_02();
	printf("test_02 ok\n");
	return 0;
}
