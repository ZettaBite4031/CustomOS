#!/bin/bash

QEMU_ARGS='-debugcon stdio -m 256 -netdev user,id=n0,hostfwd=udp:127.0.0.1:6001-172.30.233.42:6000 -device rtl8139,netdev=n0,bus=pci.0,addr=4,mac=12:34:56:78:9a:bc -device isa-debug-exit,iobase=0xf4,iosize=0x01'

if [ "$#" -le 1 ]; then
    echo "Usage: ./run.sh <image_type> <image>"
    exit 1
fi

case "$1" in
    "floppy")   QEMU_ARGS="${QEMU_ARGS} -fda $2"
    ;;
    "disk")     QEMU_ARGS="${QEMU_ARGS} -hda $2"
    ;;
    *)          echo "Unknown image type $1."
                exit 2
esac

qemu-system-i386 $QEMU_ARGS

exit $(($? >> 1))