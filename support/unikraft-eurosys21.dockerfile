# This container is used by a number of experiments in the Unikraft
# Eurosys'21 paper. It provides a ready-to-use Unikraft environments
# for our experiments, including configurations, and instrumented
# VMMs.
#
# You can easily build it with the following command:
# $ docker build -f unikraft-eurosys21.dockerfile .

# base is Debian 10
FROM hlefeuvre/instrumented-vmms

# ========================================================================
# install more packages
# ========================================================================

RUN apt-get update
RUN apt-get install -y apt-transport-https curl cmake bsdmainutils wget \
		vim git binutils autoconf automake make cmake gcc \
		g++ ca-certificates build-essential libncurses-dev \
		libyaml-dev flex git wget socat bison unzip uuid-runtime \
		python3 python3-setuptools python3-pip qemu-system-x86 \
		bridge-utils net-tools sgabios wget

# ========================================================================
# copy additional scripts
# ========================================================================

RUN wget https://raw.githubusercontent.com/unikraft/kraft/6217d48668cbdf0847c7864bc6368a6adb94f6a6/scripts/qemu-guest
RUN chmod a+x /root/qemu-guest

COPY data-unikraft-eurosys21/setup-networking.sh /root/setup-networking.sh
RUN chmod a+x /root/setup-networking.sh

# ========================================================================
# setup workspace layout
# ========================================================================

WORKDIR /root/workspace/

COPY patches-unikraft-eurosys21/ /root/workspace/patches

# create base layout
RUN mkdir -p apps libs plats

# ========================================================================
# initialize unikraft trees
# ========================================================================

RUN git clone https://github.com/unikraft/unikraft.git
RUN mv unikraft unikraft-ptsupport

RUN git clone https://github.com/unikraft/unikraft.git
RUN mv unikraft unikraft-firecracker

RUN git clone https://github.com/unikraft/unikraft.git

WORKDIR /root/workspace/unikraft

RUN git checkout 7fd6797bd5917acc515ef6ddbfa85621f4aacf5f
# CPIO support series (will be merged)
RUN patch -p1 < /root/workspace/patches/cpio-series/Minios-devel-1-5-plat-linuxu-Add-fstat-and-open-host-system-calls.diff
RUN patch -p1 < /root/workspace/patches/cpio-series/Minios-devel-2-5-plat-linuxu-Add-initrd-memory-region.diff
RUN patch -p1 < /root/workspace/patches/cpio-series/Minios-devel-3-5-lib-cpio-Create-empty-CPIO-extraction-library.diff
RUN patch -p1 < /root/workspace/patches/cpio-series/Minios-devel-4-5-lib-cpio-Add-CPIO-extraction-functionality.diff
RUN patch -p1 < /root/workspace/patches/cpio-series/Minios-devel-5-5-lib-vfscore-Add-support-to-mount-initramfs-to-root.diff
# add config options for allocators (will be merged)
RUN git apply /root/workspace/patches/registerallocs.patch
# add tracepoints instrumentation for boot time breakdown (will probably not be merged)
RUN wget -O /root/workspace/patches/statically-alloc-irqs.patch \
	https://patchwork.unikraft.org/patch/737745/raw/
RUN git apply /root/workspace/patches/statically-alloc-irqs.patch
RUN git apply /root/workspace/patches/tracepoints.patch

WORKDIR /root/workspace/unikraft-ptsupport

RUN git checkout 7fd6797bd5917acc515ef6ddbfa85621f4aacf5f
# both page table support AND tracepoints
RUN git apply /root/workspace/patches/ptsupport.patch --ignore-whitespace
RUN git apply /root/workspace/patches/statically-alloc-irqs.patch

WORKDIR /root/workspace/unikraft-firecracker

RUN git checkout a4cf6565b4cc9bacddc8628195142d5fcfc4ffeb
RUN git apply /root/workspace/patches/firecrackersupport.patch

# ========================================================================
# initialize platforms
# ========================================================================

WORKDIR /root/workspace/plats

RUN git clone https://github.com/unikraft/plat-solo5.git
RUN cd plat-solo5 && git checkout 51213c68c03208b547bfc23d02022de87cbbb696
RUN cd plat-solo5 && git apply /root/workspace/patches/solo5-instrumented.patch

# ========================================================================
# initialize libs
# ========================================================================

WORKDIR /root/workspace/libs

RUN git clone https://github.com/skuenzer/lib-tlsf.git
# use a temporary fork of the tlsf repository to workaround dead upstream
# tarball link
RUN cd lib-tlsf && git checkout skuenzer/eurosys21
RUN cd lib-tlsf && git checkout 851522e788424914a1a7763449eb7225d7a12832

RUN git clone https://github.com/unikraft/lib-mimalloc.git
RUN cd lib-mimalloc && git checkout 24089078de67be4b4709325e8479aca0c3184c23

RUN git clone https://github.com/unikraft/lib-tinyalloc.git
RUN cd lib-tinyalloc && git checkout 49f1efcce141ecc2c6d01731f1afea2d0c619eea
# this is a bug in tinyalloc, temporary fix
RUN cd lib-tinyalloc && sed -i "s/#include <uk\/tinyalloc.h>/#include <uk\/config.h>\n#include <uk\/tinyalloc.h>/" glue.c
RUN cd lib-tinyalloc && sed -i "s/ta_alloc(b, size)/ta_alloc(b, ALIGN_UP(size + 32, CONFIG_LIBTINYALLOC_ALIGNMENT))/" glue.c
RUN mv lib-tinyalloc lib-tinyalloc-patched

RUN git clone https://github.com/unikraft/lib-tinyalloc.git
RUN cd lib-tinyalloc && git checkout 49f1efcce141ecc2c6d01731f1afea2d0c619eea

RUN git clone https://github.com/unikraft/lib-newlib.git
RUN cd lib-newlib && git checkout ddc25cf1f361e33d1003ce1842212e8ff37b1e08

RUN git clone https://github.com/unikraft/lib-pthread-embedded.git
RUN cd lib-pthread-embedded && git checkout 2dd71294ab5fac328e62932992550405c866c7e8

RUN git clone https://github.com/unikraft/lib-nginx.git
RUN cd lib-nginx && git checkout d89c9a45d6a19eb71815492acb14b675e2da894a

RUN git clone https://github.com/unikraft/lib-redis.git
RUN cd lib-redis && git checkout 9dc159190204b49dba969bb37ce367e59ee6431e

RUN git clone https://github.com/unikraft/lib-lwip.git
RUN cd lib-lwip && git checkout 0330ef2df069d5d1a76abf2c3477201dbece7868
RUN cd lib-lwip && wget https://patchwork.unikraft.org/patch/737737/raw/
RUN cd lib-lwip && git apply index.html

RUN git clone https://github.com/unikraft/lib-sqlite.git
RUN cd lib-sqlite && git checkout 21ec31d578295982619a164de96b653e93e7cf9c

# ========================================================================
# initialize apps
# ========================================================================

WORKDIR /root/workspace/apps

RUN mkdir -p app-nginx app-redis app-sqlite

RUN cd app-nginx && echo "\$(eval \$(call addlib,appnginx))" > Makefile.uk
RUN cd app-redis && echo "\$(eval \$(call addlib,appredis))" > Makefile.uk
RUN cd app-sqlite && echo "\$(eval \$(call addlib,appsqlite))" > Makefile.uk

COPY data-unikraft-eurosys21/Makefile app-nginx/
RUN sed -i "s/lib-tinyalloc/lib-tinyalloc-patched/" app-nginx/Makefile
COPY data-unikraft-eurosys21/Makefile app-redis/
RUN sed -i "s/lib-tinyalloc/lib-tinyalloc-patched/" app-redis/Makefile
COPY data-unikraft-eurosys21/Makefile app-sqlite/

COPY data-unikraft-eurosys21/nginx.cpio app-nginx/
COPY data-unikraft-eurosys21/redis.cpio app-redis/

RUN cp -r app-nginx app-nginx-boottime

COPY data-unikraft-eurosys21/configs/nginx-boottime app-nginx-boottime/configs
COPY data-unikraft-eurosys21/configs/redis app-redis/configs
RUN cd app-redis && cp configs/tlsf.conf .config && make prepare && make -j
COPY data-unikraft-eurosys21/configs/sqlite app-sqlite/configs
RUN cd app-sqlite && cp configs/tlsf.conf .config && \
	make prepare && make -j
COPY data-unikraft-eurosys21/configs/nginx app-nginx/configs
RUN cd app-nginx && cp configs/tlsf.conf .config && \
	make prepare && make -j

RUN git clone https://github.com/hlef/app-sqlitebenchmark.git
RUN cd app-sqlitebenchmark && \
	git checkout 600cb7556e1d84ef906a0cb9c8a63edc46be0674
COPY data-unikraft-eurosys21/configs/sqlitebenchmark app-sqlitebenchmark/configs
RUN cd app-sqlitebenchmark && cp configs/tlsf.conf .config && \
	make prepare && make -j

RUN git clone https://github.com/unikraft/app-helloworld.git
RUN mv app-helloworld app-helloworld-boottime-pt
RUN cd app-helloworld-boottime-pt && \
	git checkout 6e079d35be30568a76ecb8f02530e21eae829340
RUN sed -i "s/unikraft/unikraft-ptsupport/" app-helloworld-boottime-pt/Makefile
COPY data-unikraft-eurosys21/configs/helloworld-boottime-pt app-helloworld-boottime-pt/configs
RUN cd app-helloworld-boottime-pt && cp configs/dyn.conf .config && \
	make prepare && make -j

RUN git clone https://github.com/unikraft/app-helloworld.git
RUN mv app-helloworld app-helloworld-boottime-fc
RUN cd app-helloworld-boottime-fc && \
	git checkout 6e079d35be30568a76ecb8f02530e21eae829340
RUN cd app-helloworld-boottime-fc && \
	git apply /root/workspace/patches/hello-ioport.patch --ignore-whitespace
RUN sed -i "s/(1500, 1)/(0x03f0, 123)/" app-helloworld-boottime-fc/main.c
RUN sed -i "s/unikraft/unikraft-firecracker/" \
	app-helloworld-boottime-fc/Makefile
COPY data-unikraft-eurosys21/configs/helloworld-boottime-fc app-helloworld-boottime-fc/configs
RUN cd app-helloworld-boottime-fc && cp configs/firecracker.conf .config && \
	make prepare && make -j

RUN git clone https://github.com/unikraft/app-helloworld.git
RUN cd app-helloworld && git checkout 6e079d35be30568a76ecb8f02530e21eae829340
RUN cd app-helloworld && \
	git apply /root/workspace/patches/hello-ioport.patch --ignore-whitespace
COPY data-unikraft-eurosys21/configs/helloworld app-helloworld/configs
RUN cd app-helloworld && cp configs/stat.conf .config && make prepare && make -j

# ========================================================================
# done!
# ========================================================================

CMD echo "Welcome to the Unikraft container!" && /bin/bash
