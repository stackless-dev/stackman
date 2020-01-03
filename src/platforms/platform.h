#pragma once
#ifndef __STACKMAN_PLATFORM_H__
#define __STACKMAN_PLATFORM_H__
/*
 * Include the proper files to define the tealet_slp_switch() function,
 * either as C source or assembler source
 */


/* Microsoft platforms */
#if defined(_M_IX86)
#include "switch_x86_msvc.h" /* MS Visual Studio on X86 */
#define STACKMAN_PLATFORM x86_msvc
#elif defined(_M_X64)
#include "switch_x86_msvc.h" /* MS Visual Studio on X64 */
#define STACKMAN_PLATFORM x64_msvc
#endif

#ifdef __GNUC__
#if defined(__amd64__)
#include "switch_x86_64_gcc.h" /* gcc on amd64 */
#define STACKMAN_PLATFORM x86_64_gcc
#elif defined(__i386__)
#include "switch_x86_gcc.h" /* gcc on X86 */
#define STACKMAN_PLATFORM x86_gcc
#elif defined(__arm__)
#include "switch_arm_gcc.h" /* gcc using arm */
#define STACKMAN_PLATFORM arm32_gcc
#elif defined(__ARM_ARCH_ISA_A64)
#include "switch_aarch64_gcc.h" /* gcc using arm aarch64*/
#define STACKMAN_PLATFORM aarch64_gcc
#endif
#endif /* __GNUC__ */

/* unless STACKMAN_OPTIONAL is defined, error if platform is not found */
#if !defined(STACKMAN_OPTIONAL) && !defined(STACKMAN_PLATFORM)
#error "Unsupported platform!"
#endif

#ifdef STACKMAN_VERBOSE
#define STRING2(X) #X
#define STRING(X) STRING2(X)
#pragma message ("Platform: " STRING(STACKMAN_PLATFORM))
#endif

#endif /*_STACKMAN_PLATFORM_H__ */
