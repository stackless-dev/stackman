#pragma once
#ifndef STACKMAN_IMPL_H
#define STACKMAN_IMPL_H

/*
 * Header to define the implementation for stackman_switch()
 * Include this file from a .c file or a .S file.  Preprocessor
 * defines:
 * STACKMAN_SWITCH_IMPL_ASM - define to 1 if file is included from a .S file
 * STACKMAN_SWITCH_STATIC - define to provide static linkage to stackman_switch()
 * 
 * See also stackman.h for main incle api
 */

#define STACKMAN_SWITCH_IMPL
#include "platforms/platform.h"
#endif