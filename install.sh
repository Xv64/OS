#!/bin/bash

echo "About to install Xv64 on your LOCAL machine."
echo "This will DESTROY the contents of /dev/sdc and assumes a boot volume mounted at /mnt/xv64_boot."
echo "If any of these assumptions are false you should STOP right now."

read -p "Do you wish to continue? [y/N] " -n 1 -r
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    exit 1
fi
echo ""
cp ./bin/kernel /mnt/xv64_boot/boot/
dd if=./bin/fs.img of=/dev/sdc
