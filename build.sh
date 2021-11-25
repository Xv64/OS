#!/bin/sh

docker build --platform linux/amd64 -t xv64:latest .
docker run --rm -v ${PWD}/bin:/src/bin -it xv64:latest
