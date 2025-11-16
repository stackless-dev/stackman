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

/* Use assembly source file */
#define STACKMAN_ASSEMBLY_SRC "stackman/platforms/switch_riscv64_gcc.S"

#endif /* STACKMAN_SWITCH_IMPL */
