#!/bin/bash

unset REBUILD IDE_MODE DEBUG EXT2
CORES=3

while getopts 'rlde' c
do
  case $c in
    r) REBUILD=TRUE ;;
    l) IDE_MODE=TRUE ;;
    d) DEBUG=TRUE ;;
    e) EXT2=TRUE ;;
  esac
done

if [ -n "$REBUILD" ]; then
    ./build.sh || exit 1
fi

DEBUG_OPTS=""
if [ -n "$DEBUG" ]; then
    CORES=2
    DEBUG_OPTS="-gdb tcp:0.0.0.0:1234 -S"
    echo "waiting on GDB..."
fi

ROOT_IMG="fs.vdi"
if [ -n "$EXT2" ]; then
    ROOT_IMG="fs-ext2.img"
fi

NETWORK="-net none"
BOOT_DISK="-hda ./bin/boot.vdi"
ROOT_DISK="-drive id=disk,file=./bin/$ROOT_IMG,if=none -device driver=ide-hd,drive=disk,bus=ahci.0"

if [ -n "$IDE_MODE" ]; then
    ROOT_DISK="-hdd ./bin/$ROOT_IMG"
fi

qemu-system-x86_64 $QEMU_OPTS $DEBUG_OPTS -device ahci,id=ahci -smp $CORES -m 512 $NETWORK $BOOT_DISK $ROOT_DISK
