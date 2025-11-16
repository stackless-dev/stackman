/*
 * RISC-V RV64 (LP64D ABI) stack switching - GCC implementation
 * 
 * This implementation uses separate assembly files (.S) for RISC-V 64-bit.
 * The LP64D ABI is used: 64-bit pointers with hardware floating point.
 */

#ifndef STACKMAN_SWITCH_IMPL
/* Just check if platform is supported */
#define STACKMAN_SWITCH_INCLUDED
#else

/* Stack pointer alignment for RISC-V - must be 16-byte aligned */
#define STACKMAN_SP_ALIGN_BYTES 16

/* Always use assembly source file for RISC-V */
#ifndef STACKMAN_ASSEMBLY_SRC
#define STACKMAN_ASSEMBLY_SRC "platforms/switch_riscv64_gcc.S"
#endif

#ifndef STACKMAN_HAVE_CALL
#define STACKMAN_HAVE_CALL 1
#define STACKMAN_STACK_ALIGN 16
#endif

#if __ASSEMBLER__ && defined(STACKMAN_ASSEMBLY_SRC)
/* Include pre-generated assembly code */
#include STACKMAN_ASSEMBLY_SRC
#endif

#endif /* STACKMAN_SWITCH_IMPL */
