/*
 * Include the proper files to define the tealet_slp_switch() function,
 * either as C source or assembler source
 * This file can be included multiple times.  Once typically to test
 * if an implementaition exists, and once (defining STACKMAN_SWITCH_IMPL)
 * to define the implementation.
 */

#undef _STACKMAN_PLATFORM
/* Microsoft platforms */
#if defined(_M_IX86)
#include "switch_x86_msvc.h" /* MS Visual Studio on X86 */
#define _STACKMAN_PLATFORM x86_msvc
#elif defined(_M_X64)
#include "switch_x86_msvc.h" /* MS Visual Studio on X64 */
#define _STACKMAN_PLATFORM x64_msvc
#endif


#ifdef __clang__
/* clang compiler */
#if defined(__amd64__)
#include "switch_x86_64_gcc.h" /* gcc on amd64 */
#define _STACKMAN_PLATFORM x86_64_clang
#elif defined(__i386__)
#include "switch_x86_gcc.h" /* gcc on X86 */
#define _STACKMAN_PLATFORM x86_clang
#endif
#endif /* __clang__ */

#if defined(__GNUC__) && !defined(__clang__)
/* real gcc */
#if defined(__amd64__)
#include "switch_x86_64_gcc.h" /* gcc on amd64 */
#define _STACKMAN_PLATFORM x86_64_gcc
#elif defined(__i386__)
#include "switch_x86_gcc.h" /* gcc on X86 */
#define _STACKMAN_PLATFORM x86_gcc
#elif defined(__arm__)
#include "switch_arm_gcc.h" /* gcc using arm */
#define _STACKMAN_PLATFORM arm32_gcc
#elif defined(__ARM_ARCH_ISA_A64)
#include "switch_aarch64_gcc.h" /* gcc using arm aarch64*/
#define _STACKMAN_PLATFORM aarch64_gcc
#endif
#endif /* __GNUC__ */

/* set STACKMAN_PLATFORM and optionally report */
#if defined(_STACKMAN_PLATFORM) && !defined(STACKMAN_PLATFORM)
#define STACKMAN_PLATFORM _STACKMAN_PLATFORM
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
