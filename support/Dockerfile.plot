# SPDX-License-Identifier: BSD-3-Clause
# Authors: Alexander Jung <a.jung@lancs.ac.uk>
FROM debian:buster

ENV DEBIAN_FRONTEND noninteractive

COPY ./requirements.txt /requirements.txt

RUN set -xe; \
    apt-get update; \
    apt-get install -y \
      git \
      python3 \
      python3-pip \
      python3-click \
      python3-tabulate \
		  python3-numpy \
      python3-graphviz \
      python3-matplotlib \
      musl-tools \
      texlive-full \
		  texlive-fonts-recommended \
      texlive-fonts-extra \
		  dvipng; \
    pip3 install -r /requirements.txt
