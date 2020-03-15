# Install instructions

Running Xv64 on real hardware is still very much a WIP. Your results will vary greatly based on your local setup. However, here is an attempt to document what has worked on some setups.

## Partitioning, Formatting & (S)ATA

Xv64 does not currently use disk partitioning, and therefore expects an entire drive for its root disk. Additionally the current version of Xv64 only supports ATA (i.e. IDE) drives - SATA support is still a work in progress. From hard disk perspective you will specifically need:

1. Two hard drives - one IDE and one anything else
    For ease of the rest of this document, lets call the IDE drive the "root disk" and the other device the "boot disk".
2. The boot disk can actually be a SATA device, and can be a partition on that device - no odd requirements for the boot disk. For the rest of this document we will assume that the boot disk will live on the first SATA device and go-exist with other operating systems.
3. Create an ext2 partition on your boot disk. This partition does not have to be large, 128MB is more than sufficient.
4. Create a `boot` directory on the boot disk and copy the Xv64 `kernel` into this path.
5. Add GRUB configuration (see the next section).
6. Write the Xv64 filesystem out to the root disk using `dd if=fs.img out=/dev/sdb` (assuming `/dev/sdb`) is root disk.

At this point you're done.

## GRUB
Using GRUB for bootloading Xv64 is easy, and useful when other operating systems are installed on the same computer. Assuming you already have GRUB installed (which you probably do if you're running Linux) then you simply need to add a new boot entry. If you do not have GRUB installed then you will need to do that first - installing GRUB is outside of the scope of this document, however.

Using a tool like `grub-customizer` add a new boot entry. In `grub-customizer`;

1. Select `other` as the type.
2. Supply "Xv64" as the name - or whatever else you desire to call this entry.
3. Assuming the first partition of the first SATA device holds the boot disk, use the following configuration:

```
insmod ext2
set root=(hd0,1)
multiboot /boot/kernel
boot
```

If the 3rd partition on the first SATA device was the boot disk, then the second line would instead read `set root=(hd0,3)`. Adjust this line as necessary to match the configuration you choose when partitioning your hard drive.

## IDE Channels & Devices

TODO: Discuss IDE channels & device limitations of Xv64.
