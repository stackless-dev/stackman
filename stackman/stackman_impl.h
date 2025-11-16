#pragma once
#ifndef STACKMAN_IMPL_H
#define STACKMAN_IMPL_H

/*
 * Header to define the implementation for stackman_switch()
 * Include this file from a .c file or a .S file.  Preprocessor
 * defines:
 * STACKMAN_LINKAGE_STATIC - define to provide static linkage to stackman_switch()
 * 
 * See also stackman.h for main include api
 */

#define STACKMAN_SWITCH_IMPL
#include "platforms/platform.h"
#endif

#if !__ASSEMBLER__
#include "stackman_switch.h"

#if STACKMAN_SWITCH_NEED_INDIRECT
STACKMAN_LINKAGE_SWITCH
void *stackman_switch(stackman_cb_t callback, void *context)
{
	/* Call through a volatile pointer to prevent inlining of stackman_switch().
	 * if stackman_switch() is implemented with in-line assembler, inlining the
	 * function may change assumptions made in the assembly code.
     * See Stackless issue 183 
     * https://github.com/stackless-dev/stackless/issues/183
     */
    static void *(*volatile stackman_switch_ptr)(stackman_cb_t callback, void *context) =
    	STACKMAN_SWITCH_INASM_NAME;
    return stackman_switch_ptr(callback, context);
}
#endif
#endif
