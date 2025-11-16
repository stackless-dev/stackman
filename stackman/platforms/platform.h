/*
 * Include the proper files to define the tealet_slp_switch() function,
 * either as C source or assembler source
 * This file can be included multiple times.  Once typically to test
 * if an implementaition exists, and once (defining STACKMAN_SWITCH_IMPL)
 * to define the implementation.
 * This results in the definition of STACKMAN_PLATFORM
 * and possibly STACKMAN_ASSEMBLY_SRC if the implentation is assembly
 * based and not in-line c.
 */

/* include once for non-impl, one additional include for impl */
#ifndef STACKMAN_PLATFORM_H_1
#define STACKMAN_PLATFORM_H_1
#define DO_STACKMAN_PLATFORM_H
#endif

#if defined(STACKMAN_SWITCH_IMPL) && !defined(STACKMAN_PLATFORM_H_2)
#define STACKMAN_PLATFORM_H_2
#ifndef DO_STACKMAN_PLATFORM_H
#define DO_STACKMAN_PLATFORM_H
#endif
#endif

#ifdef DO_STACKMAN_PLATFORM_H
#undef DO_STACKMAN_PLATFORM_H


#undef _STACKMAN_PLATFORM
#undef _STACKMAN_ABI
/* Microsoft platforms */
#if defined(_M_IX86)
#include "switch_x86_msvc.h" /* MS Visual Studio on X86 */
#define _STACKMAN_PLATFORM x86_msvc
#define _STACKMAN_ABI win_x86
#elif defined(_M_X64)
#include "switch_x64_msvc.h" /* MS Visual Studio on X64 */
#define _STACKMAN_PLATFORM x64_msvc
#define _STACKMAN_ABI win_x64
#elif defined(_M_ARM)
#include "switch_arm_msvc.h" /* MS Visual Studio on ARM */
#define _STACKMAN_PLATFORM arm_msvc
#define _STACKMAN_ABI win_arm
#elif defined(_M_ARM64)
#include "switch_arm64_msvc.h" /* MS Visual Studio on ARM */
#define _STACKMAN_PLATFORM arm64_msvc
#define _STACKMAN_ABI win_aarch64
#endif


#ifdef __clang__
/* clang compiler */
#if defined(__amd64__)
#include "switch_x86_64_gcc.h" /* gcc on amd64 */
#define _STACKMAN_PLATFORM x86_64_clang
#ifdef __APPLE__
#define _STACKMAN_ABI darwin_x86_64
#else
#define _STACKMAN_ABI sysv_amd64
#endif
#elif defined(__i386__)
#include "switch_x86_gcc.h" /* gcc on X86 */
#define _STACKMAN_PLATFORM x86_clang
#define _STACKMAN_ABI sysv_i386
#elif defined(__arm__)
#include "switch_arm_gcc.h" /* gcc using arm */
#define _STACKMAN_PLATFORM arm32_clang
#define _STACKMAN_ABI arm32
#elif defined(__ARM_ARCH_ISA_A64)
#include "switch_aarch64_gcc.h" /* gcc using arm aarch64*/
#define _STACKMAN_PLATFORM aarch64_clang
#ifdef __APPLE__
#define _STACKMAN_ABI darwin_arm64
#else
#define _STACKMAN_ABI aarch64
#endif
#endif
#endif /* __clang__ */

#if defined(__GNUC__) && !defined(__clang__)
/* real gcc */
#if defined(__amd64__)
#include "switch_x86_64_gcc.h" /* gcc on amd64 */
#define _STACKMAN_PLATFORM x86_64_gcc
#ifdef __APPLE__
#define _STACKMAN_ABI darwin_x86_64
#else
#define _STACKMAN_ABI sysv_amd64
#endif
#elif defined(__i386__)
#include "switch_x86_gcc.h" /* gcc on X86 */
#define _STACKMAN_PLATFORM x86_gcc
#define _STACKMAN_ABI sysv_i386
#elif defined(__arm__)
#include "switch_arm_gcc.h" /* gcc using arm */
#define _STACKMAN_PLATFORM arm32_gcc
#define _STACKMAN_ABI arm32
#elif defined(__ARM_ARCH_ISA_A64)
#include "switch_aarch64_gcc.h" /* gcc using arm aarch64*/
#define _STACKMAN_PLATFORM aarch64_gcc
#ifdef __APPLE__
#define _STACKMAN_ABI darwin_arm64
#else
#define _STACKMAN_ABI aarch64
#endif
#endif
#endif /* __GNUC__ */

/* set STACKMAN_PLATFORM and optionally report */
#if defined(_STACKMAN_PLATFORM) && !defined(STACKMAN_PLATFORM)
#define STACKMAN_PLATFORM _STACKMAN_PLATFORM
#define STACKMAN_ABI _STACKMAN_ABI
#ifdef STACKMAN_VERBOSE
#define STRING2(X) #X
#define STRING(X) STRING2(X)
#pragma message ("Platform: " STRING(STACKMAN_PLATFORM))
#endif
#endif

/* unless STACKMAN_OPTIONAL is defined, error if platform is not found */
#if !defined(STACKMAN_OPTIONAL) && !defined(STACKMAN_PLATFORM)
#error "Unsupported platform!"
#endif

#endif /* DO_STACKMAN_PLATFORM_H */
