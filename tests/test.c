#include "stackman.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#ifndef STACKMAN_SWITCH
#define STACKMAN_SWITCH stackman_switch
#endif

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

	sp = STACKMAN_SWITCH(&test_01_cb, &c);

	assert(sp == (void*)1);
	assert(c.sp[0] == c.sp[1]);
	assert(STACKMAN_SP_LE(c.sp[0], (void*)&c));
}


/* a more complex test, performing a long jump out of a subroutine
 * the context now stores a stub that can be jumped to 
 */
typedef struct jmp
{
	void *stack_far, *stack_near;
	void *buf;
	size_t size;
	/* every call to cb logs val in log and increments */
	int val;
	int log[10];
	int counter;
} jmp;

/* helpers to save and restore stack contents to a buffer */
void save_stack(void *sp, void *buf, size_t size)
{
	if (STACKMAN_STACK_DIR == 0)
		memcpy(buf, sp, size);
	else
		memcpy(buf, (void*)((char*)sp-size), size);
}

void restore_stack(void *sp, void *buf, size_t size)
{
	if (STACKMAN_STACK_DIR == 0)
		memcpy(sp, buf, size);
	else
		memcpy((void*)((char*)sp-size), buf, size);
}

void *jmp_cb(void* context, int opcode, void *sp)
{
	jmp *c = (jmp*)context;
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

/* Create a far jump destination. */
jmp *jmp_save(void *farptr)
{
	jmp *res = (jmp*)malloc(sizeof(jmp));
	memset(res, 0, sizeof(jmp));
	/* far end of stack, add buffer to catch memory backed registers, etc. */
	res->stack_far = STACKMAN_SP_ADD((char*)farptr, 32);

	/* first time around, get the stack pointer */
	STACKMAN_SWITCH(&jmp_cb, (void*) res);

	if (res->val == 0)
		return res;
	return 0;
}

/* execute a far jump */
void jmp_jmp(jmp *c)
{
	/* right, perform the jump to the original stack pointer */
	c->val = 1;
	STACKMAN_SWITCH(&jmp_cb, c);
}

void test_02_called(jmp *c)
{
	/* right, perform the jump to the original stack pointer */
	jmp_jmp(c);
	assert(0);
}

void test_02(void)
{
	int stack_marker;
	static jmp *c;
	jmp * tmp;

	tmp = jmp_save(&stack_marker);
	if (tmp) {
		c = tmp;
		assert(c->counter == 2);
		/* first time.  We now have the sp to return to the original one
		 * lets long jump out of a recursive function here
		 */
		assert(c->buf);
		assert(c->size);
		test_02_called(c);
		assert(0); /* never reach this */
	}

	assert (tmp == 0);
	
	assert(c->val == 1);
	assert(c->counter == 4);
}

/* test stack stuff not changing */
void test_03(void)
{
	int stack_marker;
	static jmp *c;
	jmp * tmp;
	int foo[2];

	foo[0] = 7;
	tmp = jmp_save(&stack_marker);
	if (tmp) {
		/* saved stack */
		c = tmp;
		foo[0] = 11;
		test_02_called(c);
		assert(0); /* never reach this */
	}

	assert (foo[0] == 7);
}


#if defined(STACKMAN_HAVE_CALL)
#define TEST_04
#endif
#ifdef TEST_04
/* test retrieval of stack pointer */
void *test_04_cb(void* context, int _opcode, void *sp)
{

	ctxt01 *c = (ctxt01*)context;
	stackman_op_t opcode = (stackman_op_t)_opcode;
	assert(opcode == STACKMAN_OP_CALL || opcode == 100);
	if (opcode == STACKMAN_OP_CALL)
		return test_04_cb(context, 100, sp);

	return sp; /* test return argument */
}
void test_04(void)
{
	char *block, *stack, *stack2;
	int stacksize=1024;
	int i, cnt;
	void *res;

	assert(STACKMAN_STACK_FULL_DESCENDING);

	block = (char*)malloc(stacksize);
	stack = block + stacksize;
	/* clear it all to a certain value */
	memset(block, '\x7f', stacksize);

	/* align stack properly */
	stack = (void*)STACKMAN_SP_ALIGN(stack);
	stacksize = stack-block;
	/* allocate some guard memory at the stack top */
	stack2 = stack-64;

	/* perform the call */
	res = stackman_call(test_04_cb, 0, stack2);
	assert(res==stack2);

	/* verify that the guard memory is correct */
	cnt = 0;
	for(i=0; i<64; i++)
		cnt += stack2[i] == '\x7f';
	assert(cnt == 64);
	/* verify that the memory below the stack is something else */
	cnt = 0;
	for(i=0; i<64; i++)
		cnt += stack2[-i] == '\x7f';
	assert(cnt != 64);



}

#endif


int main(int argc, char*argv[])
{
	test_01();
	printf("test_01 ok\n");
	test_02();
	printf("test_02 ok\n");

	test_03();
	printf("test_03 ok\n");
#ifdef TEST_04
	test_04();
	printf("test_04 ok\n");
#endif
	return 0;
}
