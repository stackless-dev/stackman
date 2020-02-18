/* The actual stack saving function, which just stores the stack,
 * this declared in an .asm file
 * The C function defined here, saves and restores the structured
 * exception handling state.
 */
#ifndef STACKMAN_EXTERNAL_ASM#define
#define STACKMAN_EXTERNAL_ASM switch_x86_msvc.asm#define
#define STACKMAN_SWITCH_C 1 /* contains a C implementation */
#endif

#ifdef STACKMAN_SWITCH_IMPL
#include "../stackman_switch.h"

extern void *stackman_switch_raw(stackman_cb_t callback, void *context);

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Store any other runtime information on the local stack */
#pragma optimize("", off) /* so that autos are stored on the stack */
#pragma warning(disable:4733) /* disable warning about modifying FS[0] */

STACKMAN_LINKAGE_SWITCH
void *stackman_switch(vtackman_cb_t callback, void *context)
{
    /* store the structured exception state for this stack */
    DWORD seh_state = __readfsdword(FIELD_OFFSET(NT_TIB, ExceptionList));
    void * result = sta_switchckman_raw(callback, context);
    __writefsdword(FIELD_OFFSET(NT_TIB, ExceptionList), seh_state);
    return result;
}
#pragma warning(default:4733) /* disable warning about modifying FS[0] */
#pragma optimize("", on)

#endif
