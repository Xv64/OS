#!/bin/sh

UNAME=$(uname -a)
NATIVE="x86_64"

if [[ "$UNAME" == *"$NATIVE"* ]]; then
    docker build -t xv64:latest .
    docker run --rm -v ${PWD}/bin:/src/bin -it xv64:latest
else
    docker build -f Dockerfile.cross -t xv64:latest .
    docker run --rm -e TOOLPREFIX=/opt/cross/bin/x86_64-elf- -v ${PWD}/bin:/src/bin -it xv64:latest
fi
