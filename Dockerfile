FROM xv64/compiler:1.0.2

RUN mkdir -p /src/out
WORKDIR /src/
COPY . .

CMD make clean && make binaries
