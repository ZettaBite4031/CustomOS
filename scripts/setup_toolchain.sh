#!/bin/bash

set -euo pipefail

BINUTILS_VERSION="${BINUTILS_VERSION:-2.37}"
GCC_VERSION="${GCC_VERSION:-11.2.0}"
NEWLIB_VERSION="${NEWLIB_VERSION:-4.1.0}"

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz"
NEWLIB_URL="https://sourceware.org/pub/newlib/newlib-${NEWLIB_VERSION}.tar.gz"

TARGET="${TARGET:-i686-elf}"
OPERATION="build"
JOBS="${JOBS:-$(command -v nproc >/dev/null 2>&1 && nproc || sysctl -n hw.ncpu || echo 4)}"

PREREQS_MODE="${PREREQS_MODE:-vendored}"

usage() {
    cat << EOF
        Usage: $(basename "$0") [options] <toolchains-dir>

        Options:
        -c            Clean (remove downloads/build dirs under <toolchains-dir>)
        -j <N>        Parallel jobs (default: ${JOBS})
        -t <target>   Target triple (default: ${TARGET})
        -h            Show this help

        Environment overrides:
        BINUTILS_VERSION, GCC_VERSION, NEWLIB_VERSION
        TARGET, JOBS
        CFLAGS_FOR_TARGET, CXXFLAGS_FOR_TARGET, LDFLAGS_FOR_TARGET
EOF
}

while (( "$#" )); do
  case "${1:-}" in
    -c) OPERATION="clean"; shift ;;
    -j) JOBS="${2:?}"; shift 2 ;;
    -t) TARGET="${2:?}"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    -*) echo "Unknown option: $1"; usage; exit 2 ;;
    *)  TOOLCHAINS_DIR="${1}"; shift ;;
  esac
done

if [[ -z "${TOOLCHAINS_DIR:-}" ]]; then
    echo "Missing argument: <toolchain-dir>"; usage; exit 1
fi

# Absolute paths
TOOLCHAINS_DIR="$(cd "$TOOLCHAINS_DIR" && pwd)"
PREFIX="${TOOLCHAINS_DIR}/${TARGET}-toolchain"
SYSROOT="${PREFIX}/${TARGET}"

# Work dirs
DL_DIR="${TOOLCHAINS_DIR}/_dl"
SRC_DIR="${TOOLCHAINS_DIR}/_src"
BUILD_DIR="${TOOLCHAINS_DIR}/_build"

mkdir -p "${DL_DIR}" "${SRC_DIR}" "${BUILD_DIR}"

log() { printf '\n\033[1;34m[%s]\033[0m %s\n' "$(date +%H:%M:%S)" "$*" >&2; }

need() {
    command -v "$1" >/dev/null 2>&1 || {
        echo "Missing required program: $1" >&2
        exit 1
    }
}

fetch() {
    local url="$1" out="${DL_DIR}"/$(basename "$1")
    if [[ -f "${out}" ]]; then
        log "Using cached $(basename "$out")"
    else
        log "Downloading $(basename "$out")"
        curl -fsSL "$url" -o "$out"
    fi
    printf '%s\n' "$out"
}

extract() {
  local archive="$1" dest="${SRC_DIR}"
  log "Extracting $(basename "$archive")"
  case "$archive" in
    *.tar.xz) tar -C "$dest" -xf "$archive" ;;
    *.tar.gz) tar -C "$dest" -xf "$archive" ;;
    *) echo "Unknown archive format: $archive" >&2; exit 1 ;;
  esac
}

ensure_gcc_prereqs() {
  local gcc_src="${SRC_DIR}/gcc-${GCC_VERSION}"
  if [[ ! -d "${gcc_src}" ]]; then
    printf 'FATAL: GCC source dir not found: %s\n' "${gcc_src}" >&2
    exit 1
  fi

  if [[ "${PREREQS_MODE}" == "vendored" ]]; then
    if [[ -x "${gcc_src}/contrib/download_prerequisites" ]]; then
      log "Fetching GCC prerequisites (GMP/MPFR/MPC/ISL) into source tree"
      (
        cd "${gcc_src}"
        # Write logs to stderr so stdout stays clean
        bash "./contrib/download_prerequisites" >&2
      )
    else
      printf 'FATAL: Missing contrib/download_prerequisites in %s\n' "${gcc_src}" >&2
      exit 1
    fi
  else
    log "PREREQS_MODE=system: skipping contrib/download_prerequisites"
    log "Make sure host libs are installed (e.g., gmp/mpfr/mpc dev packages)."
  fi
}

path_prepend() {
  case ":$PATH:" in
    *":$1/bin:"*) : ;;
    *) export PATH="$1/bin:$PATH" ;;
  esac
}

clean_all() {
  log "Cleaning ${TOOLCHAINS_DIR}"
  rm -rf -- "${DL_DIR}" "${SRC_DIR}" "${BUILD_DIR}" "${PREFIX}"
  log "Done."
}

need curl
need tar
need make
need gcc
need g++
need xz

for dep in gmp mpfr mpc; do
  command -v "${dep}" >/dev/null 2>&1 || {
    echo "Warning: ${dep} binary not found. Make sure ${dep} dev libraries are installed for GCC build." >&2
  }
done

if [[ "${OPERATION}" == "clean" ]]; then
  clean_all
  exit 0
fi

log "Prefix:     ${PREFIX}"
log "Target:     ${TARGET}"
log "Sysroot:    ${SYSROOT}"
log "Jobs:       ${JOBS}"
mkdir -p "${PREFIX}" "${SYSROOT}"

path_prepend "${PREFIX}"

BINUTILS_TAR="$(fetch "${BINUTILS_URL}")"
GCC_TAR="$(fetch "${GCC_URL}")"
NEWLIB_TAR="$(fetch "${NEWLIB_URL}")"

echo "${BINUTILS_TAR}"

[[ -d "${SRC_DIR}/binutils-${BINUTILS_VERSION}" ]] || extract "${BINUTILS_TAR}"
[[ -d "${SRC_DIR}/gcc-${GCC_VERSION}"       ]] || extract "${GCC_TAR}"
[[ -d "${SRC_DIR}/newlib-${NEWLIB_VERSION}" ]] || extract "${NEWLIB_TAR}"

ensure_gcc_prereqs

log "Building binutils ${BINUTILS_VERSION}"
BINUTILS_BLD="${BUILD_DIR}/binutils-${BINUTILS_VERSION}"
mkdir -p "${BINUTILS_BLD}"
pushd "${BINUTILS_BLD}" >/dev/null

"${SRC_DIR}/binutils-${BINUTILS_VERSION}/configure" \
  --target="${TARGET}" \
  --prefix="${PREFIX}" \
  --with-sysroot="${SYSROOT}" \
  --disable-nls \
  --disable-werror

make -j"${JOBS}"
make install
popd >/dev/null

path_prepend "${PREFIX}"

log "Building GCC ${GCC_VERSION} (stage1: C only, without headers)"
GCC_STAGE1_BLD="${BUILD_DIR}/gcc-stage1-${GCC_VERSION}"
mkdir -p "${GCC_STAGE1_BLD}"
pushd "${GCC_STAGE1_BLD}" >/dev/null

mkdir -p "${SYSROOT}/include"
mkdir -p "${SYSROOT}/usr/include"           # what --with-sysroot expects
mkdir -p "${SYSROOT}/sys-include"           # legacy fallback some setups use

"${SRC_DIR}/gcc-${GCC_VERSION}/configure" \
  --target="${TARGET}" \
  --prefix="${PREFIX}" \
  --with-sysroot="${SYSROOT}" \
  --with-native-system-header-dir=/include \
  --disable-nls \
  --disable-shared \
  --disable-threads \
  --disable-libmudflap \
  --disable-libssp \
  --disable-libgomp \
  --disable-libquadmath \
  --disable-multilib \
  --enable-languages=c \
  --without-headers

make -j"${JOBS}" all-gcc
make install-gcc
popd >/dev/null

log "Building newlib ${NEWLIB_VERSION}"
NEWLIB_BLD="${BUILD_DIR}/newlib-${NEWLIB_VERSION}"
mkdir -p "${NEWLIB_BLD}"
pushd "${NEWLIB_BLD}" >/dev/null

"${SRC_DIR}/newlib-${NEWLIB_VERSION}/configure" \
  --target="${TARGET}" \
  --prefix="${PREFIX}" \
  --disable-newlib-supplied-syscalls

make -j"${JOBS}"
make install
popd >/dev/null

log "Building GCC ${GCC_VERSION} (stage2: C & C++ with target libstdc++)"
GCC_STAGE2_BLD="${BUILD_DIR}/gcc-stage2-${GCC_VERSION}"
mkdir -p "${GCC_STAGE2_BLD}"
pushd "${GCC_STAGE2_BLD}" >/dev/null

export CFLAGS_FOR_TARGET="${CFLAGS_FOR_TARGET:--O2 -pipe -ffreestanding}"
export CXXFLAGS_FOR_TARGET="${CXXFLAGS_FOR_TARGET:--O2 -pipe -ffreestanding}"
export LDFLAGS_FOR_TARGET="${LDFLAGS_FOR_TARGET:-}"

"${SRC_DIR}/gcc-${GCC_VERSION}/configure" \
  --target="${TARGET}" \
  --prefix="${PREFIX}" \
  --with-sysroot="${SYSROOT}" \
  --with-native-system-header-dir=/include \
  --disable-nls \
  --disable-multilib \
  --disable-shared \
  --disable-threads \
  --disable-libmudflap \
  --disable-libssp \
  --disable-libgomp \
  --disable-libquadmath \
  --with-newlib \
  --enable-languages=c,c++

make -j"${JOBS}" all
make install
popd >/dev/null

STD_INC_DIR="${SYSROOT}/include/c++/${GCC_VERSION}"
if [[ -f "${STD_INC_DIR}/variant" && -f "${STD_INC_DIR}/optional" && -f "${STD_INC_DIR}/functional" ]]; then
  log "✔ C++ standard headers installed at:"
  echo "  ${STD_INC_DIR}"
else
  log "⚠ Could not find expected C++ headers under:"
  echo "  ${STD_INC_DIR}"
  echo "  (Check the GCC build logs above for any libstdc++ issues.)"
fi

cat <<EOF

========================================
Toolchain ready

  Target:   ${TARGET}
  Prefix:   ${PREFIX}
  Sysroot:  ${SYSROOT}

Add to your PATH:
  export PATH="${PREFIX}/bin:\$PATH"

Example compile flags (kernel, freestanding):
  ${TARGET}-g++ -std=gnu++17 -ffreestanding -fno-exceptions -fno-rtti -nostdlib \\
      -I"${SYSROOT}/include" -isystem "${SYSROOT}/include/c++/${GCC_VERSION}" \\
      -isystem "${SYSROOT}/include/c++/${GCC_VERSION}/${TARGET}" \\
      -c your_file.cpp -o your_file.o

(You can drop -fno-exceptions/-fno-rtti if you plan to use those features.)
========================================
EOF