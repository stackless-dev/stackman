#pragma once
#ifndef STACKMAN_IMPL_H
#define STACKMAN_IMPL_H

/*
 * Header to define the implementation for stackman_switch()
 * Include this file from a .c file or a .S file.  Preprocessor
 * defines:
 * STACKMAN_LINKAGE_STATIC - define to provide static linkage to stackman_switch()
 * 
 * See also stackman.h for main incle api
 */

#define STACKMAN_SWITCH_IMPL
#include "platforms/platform.h"
#endif

#if !__ASSEMBLER__
#include "stackman_switch.h"
STACKMAN_LINKAGE_SWITCH_NOINLINE
void *stackman_switch_noinline(stackman_cb_t callback, void *context)
{
#ifndef STACKMAN_EXTERNAL_ASM
	/* Use a volatile pointer to prevent inlining of stackman_switch().
     * See Stackless issue 183 
     * https://github.com/stackless-dev/stackless/issues/183
     */
    static void *(*volatile stackman_switch_ptr)(stackman_cb_t callback, void *context) = stackman_switch;
    return stackman_switch_ptr(callback, context);
#else
    /*external assembler object cannot be inlined */
    return stackman_switch(callback, context);
#endif
}

#endif