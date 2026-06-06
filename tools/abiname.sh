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

canonicalize_abi() {
	case "$1" in
		win_aarch64) printf '%s\n' "win_arm64" ;;
		*) printf '%s\n' "$1" ;;
	esac
}

detect_target_triple() {
	# Prefer querying with CFLAGS so explicit target flags (for example
	# --target=... in clang-based cross builds) are reflected.
	if ${CC} ${CFLAGS} -dumpmachine >/dev/null 2>&1; then
		${CC} ${CFLAGS} -dumpmachine
		return 0
	fi
	if ${CC} ${CFLAGS} --print-target-triple >/dev/null 2>&1; then
		${CC} ${CFLAGS} --print-target-triple
		return 0
	fi
	# Fall back to no CFLAGS in case unrelated build flags make the query fail.
	if ${CC} -dumpmachine >/dev/null 2>&1; then
		${CC} -dumpmachine
		return 0
	fi
	if ${CC} --print-target-triple >/dev/null 2>&1; then
		${CC} --print-target-triple
		return 0
	fi
	printf '%s\n' ""
}

abi_from_target_triple() {
	triple=$(printf '%s' "$1" | tr '[:upper:]' '[:lower:]')
	case "${triple}" in
		*x86_64*apple-darwin*|*x86_64*darwin*) printf '%s\n' "darwin_x86_64" ;;
		*arm64*apple-darwin*|*aarch64*apple-darwin*|*arm64*darwin*|*aarch64*darwin*) printf '%s\n' "darwin_arm64" ;;
		*x86_64*linux*) printf '%s\n' "sysv_amd64" ;;
		*i?86*linux*) printf '%s\n' "sysv_i386" ;;
		*aarch64*linux*) printf '%s\n' "aarch64" ;;
		*arm*linux*gnueabi*|*armv7*linux*|*armv6*linux*) printf '%s\n' "arm32" ;;
		*riscv64*linux*) printf '%s\n' "riscv64" ;;
		*aarch64*w64*mingw*|*arm64*windows*|*aarch64*windows*) printf '%s\n' "win_arm64" ;;
		*x86_64*w64*mingw*|*x86_64*windows*) printf '%s\n' "win_x64" ;;
		*i?86*w64*mingw*|*i?86*windows*) printf '%s\n' "win_x86" ;;
		*) printf '%s\n' "" ;;
	esac
}

mkdir -p "${here}/tmp"
# Clean up any stale temp files first
rm -f "${here}/tmp"/abiname*.c "${here}/tmp"/abiname*.c.out
tmp=$(mktemp "${here}/tmp/abinameXXX.c")

#1 create the preprocessed file
CC=${1:-cc}
CFLAGS=${2:-}
ABI_OVERRIDE=${3:-${STACKMAN_ABI:-}}
if [ -n "${ABI_OVERRIDE}" ]; then
	abi=$(canonicalize_abi "${ABI_OVERRIDE}")
	if [ -n "${STACKMAN_ABI_DEBUG:-}" ] || [ -n "${STACKMAN_ABI_TRACE:-}" ]; then
		printf '%s\n' "stackman abiname: cc=${CC} target=override-skip abi=${abi} source=override" >&2
	fi
	printf '%s\n' "${abi}"
	exit 0
fi
target_triple=$(detect_target_triple)
${CC} ${CFLAGS} -I${here}/../stackman -E -o "${tmp}" "${here}/abiname.c"
#2 compile resulting file
cc -o "${tmp}.out" "${tmp}"
#3 run it
abi=$("${tmp}.out")
abi=$(canonicalize_abi "${abi}")
target_abi=$(abi_from_target_triple "${target_triple}")
if [ -n "${target_abi}" ] && [ "${target_abi}" != "${abi}" ]; then
	printf '%s\n' "warning: stackman ABI mismatch: compiler target '${target_triple}' suggests '${target_abi}', macro probe selected '${abi}'. This usually means target intent was not fully propagated (flags/wrapper selection). Consider setting STACKMAN_ABI explicitly for deterministic packaging." >&2
fi
if [ -n "${STACKMAN_ABI_DEBUG:-}" ] || [ -n "${STACKMAN_ABI_TRACE:-}" ]; then
	printf '%s\n' "stackman abiname: cc=${CC} target=${target_triple:-unknown} abi=${abi} source=macro" >&2
fi
printf '%s\n' "${abi}"