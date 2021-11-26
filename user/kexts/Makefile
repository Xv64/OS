CC = $(TOOLPREFIX)gcc
BINPATH = ../../uobj/
FSPATH  = ../../fs

LD = $(TOOLPREFIX)ld
OBJDUMP = $(TOOLPREFIX)objdump

XFLAGS = -Werror -m64 -DX64 -mcmodel=kernel -mtls-direct-seg-refs -mno-red-zone

LDFLAGS = -m elf_x86_64 -nodefaultlibs
OPT ?= -O0

CFLAGS = -I ../../include/ -fno-canonical-system-headers -Wno-builtin-declaration-mismatch -c
CFLAGS += -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -MD -ggdb -fno-omit-frame-pointer
CFLAGS += -ffreestanding -fno-common -nostdlib -Iinclude -gdwarf-2 $(XFLAGS) $(OPT)
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

ULIB = $(BINPATH)ulib.o $(BINPATH)usys.o $(BINPATH)printf.o $(BINPATH)umalloc.o $(BINPATH)string.o
SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -c -o $@.o $@.c
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $(FSPATH)/kexts/$@ $@.o $(ULIB)
