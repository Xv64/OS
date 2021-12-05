-include local.mk

X64 ?= yes

BITS = 64
XOBJS = kobj/vm64.o
XFLAGS = -Werror -m64 -DX64 -mcmodel=kernel -mtls-direct-seg-refs -mno-red-zone
LDFLAGS = -m elf_x86_64 -nodefaultlibs
QEMU ?= qemu-system-x86_64

OPT ?= -O0

OBJS := \
	kobj/bio.o\
	kobj/ahci.o\
	kobj/console.o\
	kobj/exec.o\
	kobj/file.o\
	kobj/vfs.o\
	kobj/ide.o\
	kobj/ioapic.o\
	kobj/kalloc.o\
	kobj/kbd.o\
	kobj/lapic.o\
	kobj/log.o\
	kobj/main.o\
	kobj/mp.o\
	kobj/acpi.o\
	kobj/picirq.o\
	kobj/pipe.o\
	kobj/proc.o\
	kobj/spinlock.o\
	kobj/swtch$(BITS).o\
	kobj/syscall.o\
	kobj/sysfile.o\
	kobj/sysproc.o\
	kobj/timer.o\
	kobj/trapasm$(BITS).o\
	kobj/trap.o\
	kobj/uart.o\
	kobj/vectors.o\
	kobj/vm.o\
	kobj/vga.o\
	kobj/pci.o\
	kobj/sysstring.o\
	$(XOBJS)


K_FS_SRCS = $(wildcard kernel/fs/*.c)
K_FS_OBJS = $(patsubst %.c,%.o,$(K_FS_SRCS))

ifneq ("$(MEMFS)","")
# build filesystem image in to kernel and use memory-ide-device
# instead of mounting the filesystem on ide1
OBJS := $(filter-out kobj/ide.o,$(OBJS)) kobj/memide.o
FSIMAGE := fs.img
endif

# Cross-compiling (e.g., on Mac OS X)
# TOOLPREFIX = /opt/cross/bin/x86_64-elf-

# Using native tools (e.g., on X86 Linux)
#TOOLPREFIX =

# /usr/bin/x86_64-linux-gnu-gcc
#x /usr/bin/86_64-linux-gnu-gcc-8
CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump
CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -MD -g -ggdb -fno-omit-frame-pointer
CFLAGS += -ffreestanding -fno-common -nostdlib -Iinclude -gdwarf-2 $(XFLAGS) $(OPT)
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
ASFLAGS = -fno-pic -gdwarf-2 -Wa,-divide -Iinclude $(XFLAGS)

boot.img: out/bootblock out/kernel.elf fs.img
	# Build boot disk image
	dd if=/dev/zero of=boot.img count=10000
	dd if=out/bootblock of=boot.img conv=notrunc
	dd if=out/kernel.elf of=boot.img seek=1 conv=notrunc
	cp boot.img bin/boot.img
	qemu-img convert boot.img -O vdi bin/boot.vdi

xv6memfs.img: out/bootblock out/kernelmemfs.elf
	dd if=/dev/zero of=xv6memfs.img count=10000
	dd if=out/bootblock of=xv6memfs.img conv=notrunc
	dd if=out/kernelmemfs.elf of=xv6memfs.img seek=1 conv=notrunc

# kernel object files
kobj/%.o: kernel/%.c
	@mkdir -p kobj
	$(CC) $(CFLAGS) -c -o $@ $<

kobj/fs/%.o: kernel/fs/%.c
	@mkdir -p kobj/fs
	$(CC) $(CFLAGS) -c -o $@ $<

kobj/%.o: kernel/%.S
	@mkdir -p kobj
	$(CC) $(ASFLAGS) -c -o $@ $<

# userspace object files
uobj/%.o: user/%.c
	@mkdir -p uobj
	$(CC) $(CFLAGS) -c -o $@ $<

uobj/%.o: ulib/%.c
	@mkdir -p uobj
	$(CC) $(CFLAGS) -c -o $@ $<

uobj/unix/%.o: ulib/unix/%.c
	@mkdir -p uobj/unix
	$(CC) $(CFLAGS) -c -o $@ $<

POSIXLIB = uobj/unix/ctype.o\
		   uobj/unix/stdio.o\
		   uobj/unix/string.o\
		   uobj/unix/strings.o\
		   uobj/unix/poll.o\
		   uobj/unix/stdlib.o\

uobj/posix.o: $(POSIXLIB) ulib/usys.o
	ar rcs uobj/posix.o uobj/unix/*.o uobj/usys.o

uobj/%.o: ulib/%.S
	@mkdir -p uobj
	$(CC) $(ASFLAGS)  -c -o $@ $<

out/bootblock: kernel/bootasm.S kernel/bootmain.c
	@mkdir -p out
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -Iinclude -O -o out/bootmain.o -g -c kernel/bootmain.c
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -Iinclude -o out/bootasm.o -c kernel/bootasm.S
	$(LD) -m elf_i386 -nodefaultlibs -N -e start -Ttext 0x7C00 -o out/bootblock.o out/bootasm.o out/bootmain.o
	$(OBJDUMP) -S out/bootblock.o > out/bootblock.asm
	$(OBJCOPY) -S -O binary -j .text out/bootblock.o out/bootblock
	tools/sign.pl out/bootblock

out/entryother: kernel/entryother.S
	@mkdir -p out
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -o out/entryother.o -c kernel/entryother.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o out/bootblockother.o out/entryother.o
	$(OBJCOPY) -S -O binary -j .text out/bootblockother.o out/entryother
	$(OBJDUMP) -S out/bootblockother.o > out/entryother.asm

INITCODESRC = kernel/initcode$(BITS).S
out/initcode: $(INITCODESRC)
	@mkdir -p out
	$(CC) $(CFLAGS) -nostdinc -I. -o out/initcode.o -c $(INITCODESRC)
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o out/initcode.out out/initcode.o
	$(OBJCOPY) -S -O binary out/initcode.out out/initcode
	$(OBJDUMP) -S out/initcode.o > out/initcode.asm

ENTRYCODE = kobj/entry$(BITS).o
LINKSCRIPT = kernel/kernel$(BITS).ld
out/kernel.elf: $(OBJS) $(ENTRYCODE) out/entryother out/initcode $(LINKSCRIPT) $(FSIMAGE) $(K_FS_OBJS)
	$(LD) $(LDFLAGS) -T $(LINKSCRIPT) -o out/kernel.elf $(ENTRYCODE) $(OBJS) $(K_FS_OBJS) -b binary out/initcode out/entryother $(FSIMAGE)
	$(OBJDUMP) -S out/kernel.elf > out/kernel.asm
	$(OBJDUMP) -t out/kernel.elf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > bin/kernel.sym
	cp out/kernel.elf bin/kernel

MKVECTORS = tools/vectors$(BITS).pl
kernel/vectors.S: $(MKVECTORS)
	perl $(MKVECTORS) > kernel/vectors.S

ULIB = uobj/ulib.o uobj/usys.o uobj/printf.o uobj/umalloc.o uobj/string.o

fs/bin/%: uobj/%.o $(ULIB)
	@mkdir -p fs out fs/bin
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > out/$*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > bin/$*.sym

fs/%: uobj/%.o $(ULIB)
	@mkdir -p fs out fs/bin
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > out/$*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > bin/$*.sym

fs/forktest: uobj/forktest.o $(ULIB)
	@mkdir -p fs
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o fs/forktest uobj/forktest.o uobj/ulib.o uobj/usys.o
	$(OBJDUMP) -S fs/forktest > out/forktest.asm

out/mkfs: tools/mkfs.c include/fs.h
	gcc -Werror -Wall -o out/mkfs tools/mkfs.c

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: uobj/%.o

SUBPROGS := $(wildcard user/*/.)

FORCE:

$(SUBPROGS): uobj/posix.o $(ULIB)
	@mkdir -p fs/kexts
	$(MAKE) -C $@

UPROGS=\
	fs/bin/ps\
	fs/bin/system\
	fs/bin/cat\
	fs/bin/echo\
	fs/bin/grep\
	fs/init\
	fs/bin/kill\
	fs/bin/uptime\
	fs/bin/halt\
	fs/bin/ln\
	fs/bin/ls\
	fs/bin/mkdir\
	fs/bin/rm\
	fs/bin/sh\
	fs/bin/wc\
	fs/bin/reboot\

fs/LICENSE: LICENSE
	@mkdir -p fs
	cp LICENSE fs/LICENSE

# headers: FORCE
# 	@mkdir -p fs/src/include
# 	cp -r include fs/src

fs.img: out/mkfs fs/LICENSE $(UPROGS) $(SUBPROGS)
	find fs -type f | xargs out/mkfs fs.img $0
	touch fs.img
	cp fs.img bin/fs.img
	qemu-img convert fs.img -O vdi bin/fs.vdi

	dd if=/dev/zero of=bin/fs-ext2.img bs=1k count=5000
	mkfs -t ext2 -i 1024 -b 1024 -F bin/fs-ext2.img
	mkdir /tmp/loop
	mount -o loop bin/fs-ext2.img /tmp/loop
	cp -r ./fs/ /tmp/loop/
	umount /tmp/loop


-include */*.d

clean:
	rm -rf out fs uobj kobj
	rm -rf ./bin/*
	rm -f kernel/vectors.S boot.img xv6memfs.img fs.img .gdbinit
	#put these back...
	touch ./bin/.gitkeep
	mkdir out


binaries : fs.img boot.img
	#build fs.img & boot.img, now build vm images...
	cp Xv64.vmwarevm.tar.gz ./bin/
	cp -r fs ./bin/
	cd ./bin/ && tar -xvzf Xv64.vmwarevm.tar.gz && rm Xv64.vmwarevm.tar.gz && cd ..
	qemu-img convert boot.img -O vmdk bin/Xv64.vmwarevm/boot.vmdk
	mkdir bin/artifacts
	cp -r uobj bin/artifacts/
	cp -r kobj bin/artifacts/
	cp -r out bin/artifacts/

install: binaries
	sudo ./install.sh
