# Xv64

Xv64 is a 64-bit UNIX-like operating system targeting AMD64.

# Special Thanks

Inspired by Dennis Ritchie's and Ken Thompson's Unix Version 6 and forked from
xv6 (rev. 8) by the Massachusetts Institute of Technology, with 64-bit porting
merged from work done by Brian Swetland.

## Getting Started

Ensure that you have docker installed and execute `./build.sh` to compile the OS. Once compiled you can run it locally by either:

1. running `./run.sh` (assuming you have qemu installed), or...
2. using the compiled VMWare image `Xv64.vmwarevm` located in the `bin` folder.
3. manually setting up Virtual Box using the compiled VDI images located in `bin`.
4. installing on real hardware (instructions TBD) - see system requirements.

## System Requirements

Xv64, as it's name implies requires a 64-bit processor, specifically an AMD64 compatible one. Two or more cores/threads/sockets are required - no support for uniprocessors.

Boot environment must be legacy BIOS if using the provided bootloader. The OS can also be configured through GRUB using the ext2 extension and placing the kernel on a ext2 formatted partition (instructions TBD).

Boot device can either be a SATA or IDE disk, but the root device must be IDE and the first device/partition.

## License

This open-source software is licensed under the terms in the included `LICENSE` file.
