# Install instructions

Running Xv64 on real hardware is still very much a WIP. You're results will vary greatly based on your local setup. However, here is an attempt to document what has worked on some setups.

## Partitioning & Formatting

## GRUB
```
insmod ext2
set root=(hd0,1)
multiboot /boot/kernel
boot
```

## More?
etc
