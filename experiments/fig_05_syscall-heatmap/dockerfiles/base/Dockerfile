FROM debian:buster as build-go
MAINTAINER Gaulthier Gain <gaulthier.gain@uliege.be>
ENV DEBIAN_FRONTEND=noninteractive
ENV BUILD_DEPS 'curl git gcc patch libc6-dev libc6 ca-certificates make'
ENV INITRD No
ENV LANG en_US.UTF-8

RUN apt-get update && apt-get install -y ${BUILD_DEPS} --no-install-recommends

#INSTALL GOLAND
ENV GOVERSION 1.15
ENV GOROOT /usr/local/go
ENV GOPATH /go
ENV PATH="${GOPATH}/bin:${GOROOT}/bin:${PATH}"

RUN mkdir -p ${GOROOT}
RUN curl -OL https://golang.org/dl/go${GOVERSION}.linux-amd64.tar.gz
RUN tar -C /usr/local -xzf go${GOVERSION}.linux-amd64.tar.gz

RUN export GOPATH=${GOPATH}
RUN export GOROOT=${GOROOT}

#INSTALL toolchain
RUN mkdir -p ${GOPATH}/bin/ ${GOPATH}/pkg ${GOPATH}/src
WORKDIR ${GOPATH}/src/
RUN git clone https://github.com/gaulthiergain/tools.git
WORKDIR ${GOPATH}/src/tools/
RUN git checkout eurosys
WORKDIR ${GOPATH}/src/tools/srcs
RUN make deps
RUN make

FROM build-go as debian_env

ENV BUILD_DEPS 'wget curl make ca-certificates zip unzip xz-utils strace binutils ltrace'
RUN apt-get update && apt-get install -y ${BUILD_DEPS} --no-install-recommends

COPY --from=build-go /usr/local/go /usr/local/go
ENV GOROOT /usr/local/go
ENV GOPATH /go
ENV PATH="/go/bin:/usr/local/go/bin:${PATH}"

RUN mkdir /home/testfiles/
COPY --from=build-go ${GOPATH}/src/tools/testfiles/ /home/testfiles/
COPY --from=build-go ${GOPATH}/src/tools/srcs/tools /home/
WORKDIR /home