#!/bin/sh

if [ -z "$TOOLCHAIN_PATH" ]; then
    printf "Error: TOOLCHAIN_PATH environment variable is not set.\n Usage: TOOLCHAIN_PATH=/path/to/toolchain ./$0"
    exit 1
fi

TOOLCHAIN_PATH="${TOOLCHAIN_PATH%/}"
ARCH=$(basename "$TOOLCHAIN_PATH")
SYSROOT_PATH="${TOOLCHAIN_PATH}/${ARCH}/sysroot"

make CFLAGS="-DKINDLE -g -Os -ffunction-sections -fdata-sections" BUILDTYPE=unix-ncurses-only CC=$TOOLCHAIN_PATH/bin/$ARCH-gcc CXX=$TOOLCHAIN_PATH/bin/$ARCH-g++ AR=$TOOLCHAIN_PATH/bin/$ARCH-ar LDFLAGS="-ffunction-sections -fdata-sections -static-libstdc++ -static-libgcc -ltinfow -lrt" PKG_CONFIG_LIBDIR=$SYSROOT_PATH/usr/lib/pkgconfig:$SYSROOT_PATH/usr/share/pkgconfig PKG_CONFIG_SYSROOT_DIR=$SYSROOT_PATH bin/wordgrinder