FROM debian:10.2

RUN apt-get update
RUN apt-get -y install build-essential qemu qemu-kvm

RUN mkdir -p /src/out
WORKDIR /src/
COPY . .

CMD QEMUEXTRA=-nographic make qemu
