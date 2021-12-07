#!/bin/bash

unset REBUILD IDE_MODE DEBUG EXT2 BIG

while getopts 'rldeb' c
do
  case $c in
    r) REBUILD=TRUE ;;
    l) IDE_MODE=TRUE ;;
    d) DEBUG=TRUE ;;
    e) EXT2=TRUE ;;
    b) BIG=TRUE ;;
  esac
done

if [ -n "$REBUILD" ]; then
    ./build.sh || exit 1
fi

DEBUG_OPTS=""
if [ -n "$DEBUG" ]; then
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
CPU="-cpu Denverton -smp sockets=1 -smp cores=2 -smp threads=1"
if [ -n "$BIG" ]; then
    CPU="-cpu IvyBridge-v2 -smp sockets=2 -smp cores=12 -smp threads=2"
fi

qemu-system-x86_64 $QEMU_OPTS $DEBUG_OPTS -device ahci,id=ahci $CPU -m 512 $NETWORK $BOOT_DISK $ROOT_DISK
