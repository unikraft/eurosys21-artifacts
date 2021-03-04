FROM debian:buster

ARG UK_KRAFT_GITHUB_TOKEN=
ENV UK_KRAFT_GITHUB_TOKEN=$UK_KRAFT_GITHUB_TOKEN
ARG KRAFT_VERSION=913a31bb45cc4be6b69133473842ad3622065342
ENV DEBIAN_FRONTEND=noninteractive

RUN set -xe; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
      python3-pip \
      python3-wheel \
      python3-setuptools \
      python3-dev \
      make \
      libncursesw5-dev \
      libncursesw5 \
      libyaml-dev \
      flex \
      git \
      wget \
      patch \
      gawk \
      socat \
      bison \
      unzip \
      uuid-runtime \
      autoconf \
      build-essential \
      ca-certificates \
      locales \
      xz-utils; \
    echo "en_US.UTF-8 UTF-8" > /etc/locale.gen; \
    locale-gen; \
    pip3 install git+https://github.com/unikraft/kraft.git@$KRAFT_VERSION

env UK_WORKDIR=/usr/src/unikraft
env LC_ALL=C.UTF-8
env LANG=C.UTF-8

RUN kraft list update
