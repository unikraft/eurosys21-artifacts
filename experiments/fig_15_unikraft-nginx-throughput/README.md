# Unikraft Nginx TX with varying allocators

<img align="right" src="../../plots/fig_15_unikraft-nginx-throughput.svg" width="300" />

Similar to [`fig_13`](../fig_13_nginx-perf), we evaluate the performance with
[`wrk`](https://github.com/wg/wrk) for 1 minute using 14 threads, 30
connections, and a static 612B HTML page against various memory allocators,
including:

 * [Mimalloc](https://github.com/unikraft/lib-mimalloc);
 * [TLSF](https://github.com/skuenzer/lib-tlsf) branch: skuenzer/eurosys21;
 * [binary buddy](https://github.com/unikraft/unikraft/tree/staging/lib/ukallocbbuddy); and,
 * [tinyalloc](https://github.com/unikraft/lib-tinyalloc).

## Usage

 * `./genimages.sh` downloads and builds the tested images and takes about 5
   minutes on average;
 * `./benchmark.sh` runs the experiment and takes about 20-25 minutes on
   average; and
 * `./plot.py` is used to generate the figure.
