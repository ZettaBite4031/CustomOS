#!/bin/bash

QEMU_ARGS='-debugcon stdio -m 256 -netdev user,id=n0,hostfwd=udp:127.0.0.1:6001-172.30.233.42:6000 -device rtl8139,netdev=n0,bus=pci.0,addr=4,mac=02:CA:FE:F0:0D:1E -device isa-debug-exit,iobase=0xf4,iosize=0x01 -object filter-dump,id=n0,netdev=n0,file=network.dump'

if [ "$#" -le 1 ]; then
    echo "Usage: ./debug.sh <image_type> <image>"
    exit 1
fi

case "$1" in
    "floppy")   QEMU_ARGS="${QEMU_ARGS} -fda $PWD/$2"
    ;;
    "disk")     QEMU_ARGS="${QEMU_ARGS} -hda $PWD/$2"
    ;;
    *)          echo "Unknown image type $1."
                exit 2
esac

# b *0x7C00
# layout asm
cat > .vscode/.debug_stage2_gdb_script.gdb << EOF
    symbol-file $PWD/build/i686_debug/stage2/stage2.elf
    set disassembly-flavor intel
    target remote | qemu-system-i386 $QEMU_ARGS
EOF

gdb -x .vscode/.debug_stage2_gdb_script.gdb

