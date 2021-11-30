FROM xv64/compiler:1.0.4

RUN mkdir -p /src/out
WORKDIR /src/
COPY . .
RUN mv /src/.gdbinit /root/.gdbinit

CMD make clean && make binaries
