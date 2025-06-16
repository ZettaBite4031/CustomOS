#!/bin/bash

BINUTILS_VERSION=2.37
GCC_VERSION=11.2.0
NEWLIB_VERSION=4.1.0

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz"
NEWLIB_URL="https://sourceware.org/pub/newlib/newlib-${NEWLIB_VERSION}.tar.gz"
# ---------------------------

set -ex

TOOLCHAINS_DIR=
OPERATION='build'

while test $# -gt 0
do
    case "$1" in
        -c) OPERATION='clean'
            ;;
        *)  TOOLCHAINS_DIR="$1"
            ;;
    esac
    shift
done

if [ -z "$TOOLCHAINS_DIR" ]; then
    echo "Missing arg: toolchains directory"
    exit 1
fi

pushd "$TOOLCHAINS_DIR"

if [ "$OPERATION" = "build" ]; then

    # Download and build binutils
    BINUTILS_SRC="binutils-${BINUTILS_VERSION}"
    BINUTILS_BUILD="binutils-build-${BINUTILS_VERSION}"

    wget ${BINUTILS_URL}
    tar -xf binutils-${BINUTILS_VERSION}.tar.xz

    mkdir -p ${BINUTILS_BUILD}
    cd ${BINUTILS_BUILD} && CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS= ../binutils-${BINUTILS_VERSION}/configure \
        --prefix="${TOOLCHAIN_PREFIX}"	\
        --target=${TARGET}				\
        --with-sysroot					\
        --disable-nls					\
        --disable-werror
    make -j12 -C ${BINUTILS_BUILD}
    make -C ${BINUTILS_BUILD} install

    NEWLIB_BUILD="newlib-build-${NEWLIB_VERSION}"

    wget ${NEWLIB_URL}
    tar -xf newlib-${NEWLIB_VERSION}.tar.gz

    mkdir -p ${NEWLIB_BUILD}
    cd ${NEWLIB_BUILD} && ../newlib-${NEWLIB_VERSION}/configure \
        --target=${TARGET} \
        --prefix=${TOOLCHAIN_PREFIX}
    make -j12
    make install

    # Download and build GCC
    GCC_SRC="gcc-${GCC_VERSION}"
    GCC_BUILD="gcc-build-${GCC_VERSION}"

    wget ${GCC_URL}
    tar -xf gcc-${GCC_VERSION}.tar.xz
    mkdir -p ${GCC_BUILD}
    cd ${GCC_BUILD} && CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS= ../gcc-${GCC_VERSION}/configure \
        --prefix="${TOOLCHAIN_PREFIX}" 	\
        --target=${TARGET}				\
        --disable-nls					\
        --enable-languages=c,c++		\
        --with-newlib
    make -j12 -C ${GCC_BUILD} all-gcc all-target-libgcc
    make -C ${GCC_BUILD} install-gcc install-target-libgcc

elif [ "$OPERATION" = "clean" ]; then
	rm -rf *
fi
