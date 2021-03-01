#!/bin/bash

# install dependencies
apt -t buster-backports install golang

# build firectl
if [ ! -d "firectl" ]; then
	wget https://github.com/firecracker-microvm/firectl/archive/v0.1.0.tar.gz
	tar -xf v0.1.0.tar.gz
	rm v0.1.0.tar.gz
	pushd firectl-0.1.0
	make && cp firectl /usr/local/bin/
	popd
fi

# build wrk
if [ ! -d "wrk" ]; then
	wget https://github.com/wg/wrk/archive/4.1.0.tar.gz
	tar -xf 4.1.0.tar.gz
	rm 4.1.0.tar.gz
	pushd wrk-4.1.0
	make && cp wrk /usr/local/bin/
	popd
fi

# install unikraft gdb script for tracepoints
if [ ! -d "unikraft-scripts" ]; then
	mkdir -p unikraft-scripts
	pushd unikraft-scripts
	git clone https://github.com/unikraft/unikraft.git
	pushd unikraft
	git checkout 7fd6797bd5917acc515ef6ddbfa85621f4aacf5f
	popd
	popd
	echo "source $(pwd)/unikraft-scripts/unikraft/support/scripts/uk-gdb.py" \
		>> ~/.gdbinit
fi
