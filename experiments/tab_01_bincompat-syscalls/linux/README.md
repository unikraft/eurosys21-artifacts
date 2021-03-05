The kernel image in this directory is used for the evaluation of the Linux
kernel experiments. It was created from the following source:

https://github.com/skuenzer/linux/ Branch: skuenzer/syscallbench

Two additional commits are applied to a vanilla 5.11 upstream kernel
(kernel.org). These patches introduce a `noop` system call with the
number 500, which returns only a constant (`0xC0FFEE`)`. This is used
to evaluate the baseline costs of entering and exiting a system call
handler.

We used the build configuration file `config-5.11.0-sysnoop+` for
compiling. It is derived from a standard Debian Buster 5.10 kernel
configuration.

In order to compile the kernel by yourself, please run the following
commands:

``` shell
git clone --branch skuenzer/eurosys21 https://github.com/skuenzer/linux/
cd linux/
cp <path-to>/config-5.11.0-sysnoop+ .config
make oldconfig
make -j
```
