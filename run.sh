#!/bin/bash

unset REBUILD IDE_MODE DEBUG
CORES=3

while getopts 'rld' c
do
  case $c in
    r) REBUILD=TRUE ;;
    l) IDE_MODE=TRUE ;;
    d) DEBUG=TRUE ;;
  esac
done

if [ -n "$REBUILD" ]; then
    ./build.sh || exit 1
fi

DEBUG_OPTS=""
if [ -n "$DEBUG" ]; then
    CORES=1
    DEBUG_OPTS="-gdb tcp:0.0.0.0:1234 -S"
    echo "waiting on GDB..."
fi

NETWORK="-net none"
BOOT_DISK="-hda ./bin/boot.vdi"
ROOT_DISK="-drive id=disk,file=./bin/fs.vdi,if=none -device driver=ide-hd,drive=disk,bus=ahci.0"

if [ -n "$IDE_MODE" ]; then
    ROOT_DISK="-hdd ./bin/fs.vdi"
fi

qemu-system-x86_64 $QEMU_OPTS $DEBUG_OPTS -device ahci,id=ahci -smp $CORES -m 512 $NETWORK $BOOT_DISK $ROOT_DISK
