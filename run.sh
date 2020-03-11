#!/bin/sh

# docker-compose build xv6
# docker-compose run xv6

qemu-system-x86_64 -net none -hda ./bin/boot.vdi -drive id=disk,file=./bin/fs.vdi,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0 -smp 3 -m 512 -nographic
