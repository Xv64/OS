# Xv64

Xv64 is a 64-bit operating system targeting AMD64 compatible processors, featuring an SMP hybrid microkernel, POSIX/UNIX compatibility layer, AHCI/SATA support, & much more. The goal of Xv64 is to implement a clean, modern, 64-bit only OS that can both maintain a POSIX compatibility layer while also exploring unique OS design concepts.

![](docs/pics/Xv64-2020-03-24.gif)


## Getting Started / TL;DR

To get started hacking away on Xv64, you'll need a few requirements:

1. qemu
2. Docker†


Once you have those installed, execute:

`env QEMU_OPTS=-nographic ./run.sh -r`

This will build the OS from source, and run it within qemu on your system.

###### 1. QEMU

QEMU is perhaps the most tested option for running Xv64 within a virtual environment. For ease of use a helper script called `run.sh` is provided. This script takes a variety of parameters:

`-r` - this parameter rebuilds the app. This is not needed if the contents of `bin` are present or otherwise do not need to be rebuilt. This can also be omitted if you prefer not to use Docker, and instead run `make binaries` and the the `run.sh` script WITHOUT the `-r` parameter. conversely, if you supply the `-r` parameter you do NOT need to manually invoke `./build.sh`.

`-e` - this parameter stands for "ext2" and enables EXT2 drives. At the moment EXT2 drives are a work-in-progress, so if you actually want to run the OS (as opposed to developing EXT2 support) you will want to EXCLUDE this parameter.

Additional QEMU arguments can be supplied with the `QEMU_OPTS` environmental variable. For example, if you wanted to supply the QEMU option to disable its graphical UI (`-nographic`), you'd do that like:

`env QEMU_OPTS=-nographic ./run.sh -r`

###### 2. Real hardware

Xv64 supports running directly on REAL hardware. If you desire to go this route please see the system requirements below and read the [INSTALL.md](INSTALL.md) document for details.


† you do NOT need Docker to build or run Xv64. Provided the correct dependencies are installed, you can run `make` directly from your host OS - most likely Debian Linux. The Docker configuration is provided for ease of setup, please refer to the Dockerfile for details of what system dependencies (like `build-essential`, etc) are required if you forgo Docker.

## System Requirements

Xv64, as it's name implies requires a 64-bit processor, specifically an AMD64 compatible one like those manufactured by Intel or AMD since approximately 2003/2004. Two or more cores/threads/sockets are required - no support for uniprocessors.

Boot environment must be legacy BIOS if using the provided bootloader. The OS can also be configured through GRUB using the ext2 extension and placing the kernel on a ext2 formatted partition.

See [INSTALL.md](INSTALL.md) for additional requirements and specific installation steps.

## Special Thanks

Inspired by Dennis Ritchie's and Ken Thompson's Unix Version 6 and forked from
xv6 (rev. 8) by the Massachusetts Institute of Technology, with 64-bit porting
merged from work done by Brian Swetland.

## License

This open-source software is licensed under the terms in the included [LICENSE](LICENSE)  file.
