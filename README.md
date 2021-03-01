IMPORTANT: connect to the server using `ssh -X` to enable X11 forwarding
(otherwise rumprun experiments will fail!).

IMPORTANT: always run scripts in their own directory!

# System requirements

```
apt install python3 python3-click python3-tabulate \
		git python3-numpy python3-matplotlib \
		texlive-fonts-recommended texlive-fonts-extra \
		dvipng qemu-system-x86 redis-utils socat \
		uuid-runtime bridge-utils net-tools
```

The docker daemon has to be up and running (instructions [here](https://docs.docker.com/engine/install/debian/)).

qemu-guest has to be installed:

```
mv tools/qemu-guest /usr/local/bin
```

firectl, wrk, and unikraft scripts have to be installed:

```
cd tools/ && ./build-toolchain.sh
```
