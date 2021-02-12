FROM alpine:3.13 as build-base
RUN apk update && apk add cmake build-base pkgconfig zeromq-dev
COPY source/ /project/source
RUN mkdir /project/build
WORKDIR /project/build
RUN sed -i 's/sys\/signal.h/signal.h/g' /project/source/signal/server.c
RUN sed -i 's/sys\/signal.h/signal.h/g' /project/source/signal/client.c
RUN cmake ../source -DCMAKE_EXPORT_COMPILE_COMMANDS=1
RUN make

FROM build-base as debug
RUN apk add gdb
WORKDIR /project/build
RUN cmake ../source -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=debug
RUN make

FROM alpine:3.13 as ipc-clients
RUN apk update && apk add libzmq
COPY --from=build-base /project/build/zeromq/zeromq-client /
COPY --from=build-base /project/build/tcp/tcp-client /

FROM alpine:3.13 as ipc-servers
RUN apk update && apk add libzmq
COPY --from=build-base /project/build/zeromq/zeromq-server /
COPY --from=build-base /project/build/tcp/tcp-server /

