#!/bin/sh

# docker-compose build xv6
# docker-compose run xv6

NETWORK="-net none"
BOOT_DISK="-hda ./bin/boot.vdi"
ROOT_DISK="-drive id=disk,file=./bin/fs.vdi,if=none -device ide-drive,drive=disk,bus=ahci.0"

qemu-system-x86_64 -device ahci,id=ahci -smp 3 -m 512 -nographic $NETWORK $BOOT_DISK $ROOT_DISK
