CC = $(TOOLPREFIX)gcc
BINPATH = ../../uobj/
FSPATH  = ../../fs

CFLAGS = -I ../../include/unix/ -fno-canonical-system-headers -Wno-builtin-declaration-mismatch -c
CFLAGS += -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m64 -fno-omit-frame-pointer
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# lisp:
# 	$(CC) $(CFLAGS) lisp.c -o lisp.o
#
# 	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $(BINPATH)$@ *.o $(BINPATH)posix.o
#
# 	cp lisp.o $(FSPATH)/bin/lisp

lisp:
		echo "skipping lisp"
