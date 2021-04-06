#!/bin/sh

# this script compiles and runs stackman/abiname.c which merely prints
# out the name of the abi.  This can be used by makefiles to identify
# the correct library path to use to link the library
# Instead of just compiling and running, we will use the provided compiler
# and flags to just invoke the pre-processor.  We then use the default
# compiler and linker to compile and link it.  This ensures that the 
# script works in cross-compilation environments and can actually
# run the provided code.
set -eu
here=$(dirname "$0")
mkdir -p "${here}/tmp"
tmp=$(mktemp "${here}/tmp/abinameXXX.c")

#1 create the preprocessed file
CC=${1:-cc}
CFLAGS=${2:-}
${CC} ${CFLAGS} -E -o "${tmp}" "${here}/stackman/abiname.c"
#2 compile resulting file
cc -o "${tmp}.out" "${tmp}"
#3 run it
"${tmp}.out"