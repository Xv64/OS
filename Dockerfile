FROM debian:10.2

RUN apt-get update
RUN apt-get -y install build-essential qemu wget qemu-utils
# RUN apt-get -y install gcc-x86-64-linux-gnu
# RUN apt-get -y install gcc-8-x86-64-linux-gnu

RUN mkdir -p /opt/build
RUN mkdir -p /opt/cross
WORKDIR /opt/build
RUN wget https://gnu.askapache.com/binutils/binutils-2.32.tar.xz
RUN wget http://mirrors.concertpass.com/gcc/releases/gcc-8.3.0/gcc-8.3.0.tar.gz

ENV PREFIX="/opt/cross"
ENV TARGET=x86_64-elf
ENV PATH="$PREFIX/bin:$PATH"

RUN tar -xvf binutils-2.32.tar.xz
RUN tar -xvzf gcc-8.3.0.tar.gz

RUN cd /opt/build/binutils-2.32

# compile binutils
RUN mkdir -p /opt/build/build-binutils
WORKDIR build-binutils
RUN ../binutils-2.32/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
RUN make
RUN make install

# compile gcc
RUN apt-get install -y bison flex libgmp-dev libmpc-dev libmpfr-dev texinfo libisl-dev
RUN mkdir -p /opt/build/build-gcc
WORKDIR /opt/build/build-gcc
RUN ../gcc-8.3.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
RUN make all-gcc
RUN make all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 -mcmodel=kernel -mno-red-zone' || true
# will fail with: cc1: error: code model kernel does not support PIC mode
RUN sed -i 's/PICFLAG/DISABLED_PICFLAG/g' $TARGET/libgcc/Makefile
RUN make all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 -mcmodel=kernel -mno-red-zone'
# RUN make all-target-libgcc
RUN make install-gcc
RUN make install-target-libgcc


RUN mkdir -p /src/out
WORKDIR /src/
COPY . .

CMD make clean && make binaries
