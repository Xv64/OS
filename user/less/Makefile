TOOLPREFIX = /opt/cross/bin/x86_64-elf-

CC = $(TOOLPREFIX)gcc
BINPATH = ../../uobj/
FSPATH  = ../../fs

CFLAGS = -I ../../include/unix/ -fno-canonical-system-headers -Wno-builtin-declaration-mismatch -c
CFLAGS += -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m64 -fno-omit-frame-pointer
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# less:
# 	$(CC) $(CFLAGS) less.c -o less.o
#
# 	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $(BINPATH)$@ *.o $(BINPATH)posix.o $(BINPATH)ulib.o $(BINPATH)printf.o $(BINPATH)usys.o $(BINPATH)umalloc.o
#
# 	cp less.o $(FSPATH)/less

less:
	echo "skipping 'less'"
