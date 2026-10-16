#include "stackman.h"

#include <assert.h>
#include <fenv.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Name of the test currently running, so a fault names itself.
 * stdout from a crashed process is lost when it is fully buffered. */
static const char *current_test;

static LONG WINAPI test_crash(EXCEPTION_POINTERS *info)
{
	EXCEPTION_RECORD *rec = info->ExceptionRecord;

	fprintf(stderr, "exception 0x%08lx at %p during %s",
		rec->ExceptionCode, rec->ExceptionAddress,
		current_test ? current_test : "startup");
	if (rec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION &&
	    rec->NumberParameters >= 2) {
		fprintf(stderr, " %s %p",
			rec->ExceptionInformation[0] ? "write" : "read",
			(void *)rec->ExceptionInformation[1]);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
	return EXCEPTION_CONTINUE_SEARCH;
}
#endif

static void test_startup(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
#ifdef _WIN32
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
	AddVectoredExceptionHandler(1, test_crash);
#endif
}

static void test_enter(const char *name)
{
#ifdef _WIN32
	current_test = name;
#endif
	printf("%s\n", name);
}

/* Soft-float arm32 does not spill FPSCR. Softfp selects that assembly. */
#if defined(__arm__) && !defined(__aarch64__) && !defined(__ARM_PCS_VFP)
#define STACKMAN_ROUND_ISOLATED 0
#else
#define STACKMAN_ROUND_ISOLATED 1
#endif


#define assert_align(p) assert((intptr_t)(p) == STACKMAN_SP_ALIGN(p))

/* simple test without modifying stack pointers.
 * test that the callback is called with valid
 * stack pointers and the stage member in the
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
	assert_align(sp);
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
		/* RESTORE must receive the active switched stack pointer. */
		assert(sp == c->stack_near);
		restore_stack(c->stack_near, c->buf, c->size);
		return sp;
	}
}

/* Create a far jump destination. */
jmp *jmp_save(void *farptr)
{
	jmp *res = (jmp*)malloc(sizeof(jmp));
	assert(res);
	memset(res, 0, sizeof(jmp));

	/* add safety margin */
	/* far end of stack, add buffer to catch memory backed registers, etc. */
	res->stack_far = STACKMAN_SP_ADD((char*)farptr, 32);

	/* first time around, get the stack pointer */
	stackman_switch(&jmp_cb, (void*) res);

	if (res->val == 0)
		return res;
	return 0;
}

/* execute a far jump */
void jmp_jmp(jmp *c)
{
	/* right, perform the jump to the original stack pointer */
	c->val = 1;
	stackman_switch(&jmp_cb, c);
}

void test_02_called(jmp *c)
{
	/* right, perform the jump to the original stack pointer */
	jmp_jmp(c);
	assert(0);
}

void test_02(void *stack_marker)
{
	static jmp *c;
	jmp * tmp;

	tmp = jmp_save(stack_marker);
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
void test_03(void *stack_marker)
{
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


/* Sentinel returned by stackman_call. The epilogue must leave it intact. */
static void *const stackman_call_result = (void *)(intptr_t)0x1234;

/* test retrieval of stack pointer */
void *test_04_cb(void* context, int _opcode, void *old_sp)
{
	unsigned char scratch[256];

	assert_align(old_sp);
	ctxt01 *c = (ctxt01*)context;
	stackman_op_t opcode = (stackman_op_t)_opcode;
	assert(opcode == STACKMAN_OP_CALL || opcode == 100);
	/* Write the stack the callback is running on. The caller's locals
	 * must still be intact when stackman_call returns. */
	if (opcode == STACKMAN_OP_CALL) {
		memset(scratch, 0xa5, sizeof scratch);
		assert(scratch[0] == 0xa5 && scratch[sizeof scratch - 1] == 0xa5);
		/* one level of recursion here so that we actually use some of the new stack */
		test_04_cb(context, 100, old_sp);
	}

	c->sp[0] = &c;
	c->sp[1] = old_sp;
	return stackman_call_result;
}

/* test stackman_call() with a non-null stack pointer */
void test_04(void)
{
	char *block, *stack, *stack2;
	/* The callback recurses once. An MSVC Debug frame (/ZI) is several
	 * hundred bytes, so two of them overflow a 1KB block into the heap. */
	intptr_t stacksize = 8192;
	int i, cnt;
	int caller_local = 0x51a7;
	void *result;
	ctxt01 ctxt;

	assert(STACKMAN_STACK_FULL_DESCENDING);

	block = (char*)malloc(stacksize);
	assert(block);
	stack = block + stacksize;
	/* clear it all to a certain value */
	memset(block, '\x7f', stacksize);

	/* align stack properly */
	stack = (void*)STACKMAN_SP_ALIGN(stack);
	stacksize = stack-block;
	/* allocate some guard memory at the stack top */
	stack2 = stack-64;

	/* perform the call */
	result = stackman_call(test_04_cb, &ctxt, stack2);
	assert(result == stackman_call_result);
	assert(caller_local == 0x51a7);

	/* verify that the called function saw stack in the right range */
	assert(STACKMAN_SP_LE(block, ctxt.sp[0]));
	assert(STACKMAN_SP_LS(ctxt.sp[0], stack2));

	/* The pointer passed to the callback is the previous stack pointer.
	 * It is not the stack given to stackman_call, and it does not fall
	 * inside the new block [block, stack). */
	assert(ctxt.sp[1] != (void *)stack2);
	assert(STACKMAN_SP_LS(ctxt.sp[1], block) ||
	       STACKMAN_SP_LE((void *)stack, ctxt.sp[1]));

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

/* test stackman_call() with a null stack pointer */
void test_05(void)
{
	ctxt01 ctxt;
	int caller_local = 0x51a7;
	void *result;

	assert(STACKMAN_STACK_FULL_DESCENDING);

	/* perform the call */
	result = stackman_call(test_04_cb, &ctxt, 0);
	assert(result == stackman_call_result);
	assert(caller_local == 0x51a7);

	/* A null stack leaves the caller in place. The callback receives
	 * that live stack pointer, which is at or below the caller's locals. */
	assert(ctxt.sp[1]);
	assert(STACKMAN_SP_LE(ctxt.sp[0], ctxt.sp[1]));
	assert(STACKMAN_SP_LE(ctxt.sp[1], (void *)&caller_local));
	assert(STACKMAN_SP_LE(ctxt.sp[1], &ctxt));
}

/* Test our various macros */
void test_06()
{

	int local=0;
	void *away = STACKMAN_SP_FURTHEST;
	void *close = STACKMAN_SP_NEAREST;
	assert(STACKMAN_SP_LE(&local, away));
	assert(STACKMAN_SP_LS(&local, away));
	assert(STACKMAN_SP_LE(close, &local));
	assert(STACKMAN_SP_LS(close, &local));

	assert(STACKMAN_SP_LE(&local, &local));
	assert(!STACKMAN_SP_LS(&local, &local));

	assert((void*)STACKMAN_SP_ALIGN(away) == away);

}

/* Test floating point register preservation across stack switches.
 * This test verifies that callee-saved FP registers are properly
 * preserved across stack switches on all platforms.
 * 
 * Uses arrays to encourage compiler to use multiple FP registers
 * and potentially SIMD instructions where available.
 */
typedef struct fp_test_context {
	void *saved_sp;
	int switch_done;
} fp_test_context;

void *test_07_cb(void* context, int opcode, void *sp)
{
	fp_test_context *c = (fp_test_context*)context;
	
	if (opcode == STACKMAN_OP_SAVE) {
		c->saved_sp = sp;
		return sp; /* No stack switch */
	} else {
		c->switch_done = 1;
		return sp;
	}
}

/* Aggressive FP computation using arrays to encourage vectorization
 * and use of multiple FP registers including SIMD if available
 */
void test_07_compute(double *input, double *output, int count)
{
	int i;
	/* Multiple operations to stress FP register usage */
	for (i = 0; i < count; i++) {
		double a = input[i];
		double b = a * 1.234567;
		double c = b + 0.987654;
		double d = c * c;
		double e = d - a;
		output[i] = e / (a + 1.0);
	}
}

void test_07(void)
{
	fp_test_context ctx;
	double input[16], output1[16], output2[16];
	int i;
	
	/* Initialize input with varying values */
	for (i = 0; i < 16; i++) {
		input[i] = (i + 1) * 3.14159 + 0.123456 * i;
	}
	
	/* First computation - fills FP registers */
	test_07_compute(input, output1, 16);
	
	/* Additional FP work to maximize register pressure */
	double sum1 = 0.0, sum2 = 0.0;
	for (i = 0; i < 16; i++) {
		sum1 += output1[i] * (i + 1);
		sum2 += input[i] / (i + 2);
	}
	
	/* Stack switch - all FP registers must be preserved */
	memset(&ctx, 0, sizeof(ctx));
	stackman_switch(&test_07_cb, &ctx);
	assert(ctx.switch_done);
	
	/* Second computation - should produce identical results */
	test_07_compute(input, output2, 16);
	
	/* Verify outputs match exactly */
	for (i = 0; i < 16; i++) {
		assert(output1[i] == output2[i]);
	}
	
	/* Verify accumulated sums still work */
	double sum3 = 0.0, sum4 = 0.0;
	for (i = 0; i < 16; i++) {
		sum3 += output2[i] * (i + 1);
		sum4 += input[i] / (i + 2);
	}
	assert(sum1 == sum3);
	assert(sum2 == sum4);
}

/* Test integer register preservation across stack switches.
 * This test verifies that callee-saved integer registers are properly
 * preserved across stack switches on all platforms.
 * 
 * Uses complex integer arithmetic with many intermediate values to
 * pressure the compiler to use multiple registers.
 */
typedef struct int_test_context {
	void *saved_sp;
	int switch_done;
} int_test_context;

void *test_08_cb(void* context, int opcode, void *sp)
{
	int_test_context *c = (int_test_context*)context;
	
	if (opcode == STACKMAN_OP_SAVE) {
		c->saved_sp = sp;
		return sp; /* No stack switch */
	} else {
		c->switch_done = 1;
		return sp;
	}
}

/* Complex integer computation with many intermediate values
 * to encourage use of multiple callee-saved registers
 */
void test_08_compute(long *input, long *output, int count)
{
	int i;
	/* Multiple operations creating intermediate values to stress register usage */
	for (i = 0; i < count; i++) {
		long a = input[i];
		long b = a * 123456789L;
		long c = b + 987654321L;
		long d = c ^ (a << 3);
		long e = d - (a * 7);
		long f = e | (b & 0xFF00FF00);
		long g = f + (c >> 2);
		long h = g ^ (d * 13);
		output[i] = h - (a + b + c);
	}
}

void test_08(void)
{
	int_test_context ctx;
	long input[16], output1[16], output2[16];
	int i;
	
	/* Initialize input with varying values */
	for (i = 0; i < 16; i++) {
		input[i] = (long)(i + 1) * 314159L + i * 271828L;
	}
	
	/* First computation - fills integer registers */
	test_08_compute(input, output1, 16);
	
	/* Additional integer work to maximize register pressure */
	long sum1 = 0, sum2 = 0, xor1 = 0;
	for (i = 0; i < 16; i++) {
		sum1 += output1[i] * (i + 1);
		sum2 += input[i] << (i & 3);
		xor1 ^= (output1[i] + input[i]);
	}
	
	/* Stack switch - all callee-saved integer registers must be preserved */
	memset(&ctx, 0, sizeof(ctx));
	stackman_switch(&test_08_cb, &ctx);
	assert(ctx.switch_done);
	
	/* Second computation - should produce identical results */
	test_08_compute(input, output2, 16);
	
	/* Verify outputs match exactly */
	for (i = 0; i < 16; i++) {
		assert(output1[i] == output2[i]);
	}
	
	/* Verify accumulated values still work */
	long sum3 = 0, sum4 = 0, xor2 = 0;
	for (i = 0; i < 16; i++) {
		sum3 += output2[i] * (i + 1);
		sum4 += input[i] << (i & 3);
		xor2 ^= (output2[i] + input[i]);
	}
	assert(sum1 == sum3);
	assert(sum2 == sum4);
	assert(xor1 == xor2);
}

/* The caller's rounding mode comes back from stackman_switch when the
 * ABI spills the floating-point control register. The callback's change
 * is still visible on STACKMAN_OP_RESTORE: the reload happens after
 * that callback returns. stackman_call does not spill the register.
 * Soft-float arm32 keeps the callback's mode.
 */
void *test_09_cb(void *context, int opcode, void *sp)
{
	int *saw = (int *)context;

	assert_align(sp);
	if (opcode == (int)STACKMAN_OP_SAVE) {
		assert(fegetround() == FE_DOWNWARD);
		assert(fesetround(FE_TOWARDZERO) == 0);
		*saw = 1;
		return sp;
	}
	assert(opcode == (int)STACKMAN_OP_RESTORE);
	assert(fegetround() == FE_TOWARDZERO);
	return sp;
}

void test_09(void *stack_marker)
{
	static jmp *saved;
	int previous = fegetround();
	int saw = 0;
	jmp *created;

	assert(fesetround(FE_DOWNWARD) == 0);
	assert(fegetround() == FE_DOWNWARD);
	stackman_switch(&test_09_cb, &saw);
	assert(saw == 1);
	if (STACKMAN_ROUND_ISOLATED)
		assert(fegetround() == FE_DOWNWARD);
	else
		assert(fegetround() == FE_TOWARDZERO);

	/* The saved stack carries FE_DOWNWARD. This activation then
	 * switches away with FE_UPWARD live. Resuming the copy must
	 * see the mode that was saved with it. */
	assert(fesetround(FE_DOWNWARD) == 0);
	created = jmp_save(stack_marker);
	if (created) {
		saved = created;
		assert(fegetround() == FE_DOWNWARD);
		assert(fesetround(FE_UPWARD) == 0);
		jmp_jmp(saved);
		assert(0);
	}

	if (STACKMAN_ROUND_ISOLATED)
		assert(fegetround() == FE_DOWNWARD);
	else
		assert(fegetround() == FE_UPWARD);
	assert(fesetround(previous) == 0);
	if (saved) {
		free(saved->buf);
		free(saved);
		saved = 0;
	}
}

int main(int argc, char*argv[])
{
	int stack_marker = 0;

	test_startup();
	test_enter("test_01");
	test_01();
	printf("test_01 ok\n");
	test_enter("test_02");
	test_02((void*)&stack_marker);
	printf("test_02 ok\n");

	test_enter("test_03");
	test_03((void*)&stack_marker);
	printf("test_03 ok\n");
	test_enter("test_04");
	test_04();
	printf("test_04 ok\n");
	test_enter("test_05");
	test_05();
	printf("test_05 ok\n");
	test_enter("test_06");
	test_06();
	printf("test_06 ok\n");
	test_enter("test_07");
	test_07();
	printf("test_07 ok\n");
	test_enter("test_08");
	test_08();
	printf("test_08 ok\n");
	test_enter("test_09");
	test_09((void*)&stack_marker);
	printf("test_09 ok\n");
	return 0;
}
