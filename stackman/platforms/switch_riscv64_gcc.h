/*
 * RISC-V RV64 (LP64D ABI) stack switching - GCC implementation
 * 
 * This implementation uses separate assembly files (.S) for RISC-V 64-bit.
 * The LP64D ABI is used: 64-bit pointers with hardware floating point.
 */

/* Always use assembly source file for RISC-V */
#if !defined(STACKMAN_ASSEMBLY_SRC)
#define STACKMAN_ASSEMBLY_SRC "platforms/switch_riscv64_gcc.S"
#endif

/* Visible from a normal #include "stackman.h". stackman.h otherwise
 * leaves STACKMAN_STACK_ALIGN at 4. */
#ifndef STACKMAN_STACK_ALIGN
#define STACKMAN_STACK_ALIGN 16
#endif

#ifndef STACKMAN_SWITCH_IMPL
/* Just check if platform is supported */
#define STACKMAN_SWITCH_INCLUDED
#else

#if __ASSEMBLER__ && defined(STACKMAN_ASSEMBLY_SRC)
/* Include pre-generated assembly code */
#include STACKMAN_ASSEMBLY_SRC
#endif

#endif /* STACKMAN_SWITCH_IMPL */
