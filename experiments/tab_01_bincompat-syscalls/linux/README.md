The kernel image in this directory is used for the evaluation of the Linux
kernel experiments. It was created from the following source:

https://github.com/skuenzer/linux/ Branch: skuenzer/syscallbench

Two additional commits are applied to a vanilla 5.11 upstream kernel
(kernel.org). These patches introduce a `noop` system call with the
number 500, which returns only a constant (`0xC0FFEE`)`. This is used
to evaluate the baseline costs of entering and exiting a system call
handler.

The build configuration file can be found in this directory, too. It is
based on a Debian Buster 5.10 kernel configuration.
