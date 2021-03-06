# NGINX throughput comparison

<img align="right" src="../../plots/fig_13_nginx-perf.svg" width="300" />

We measure the throughput of [NGINX](nginx.org/) in a wide range of
systems, including:

 * [HermiTux](https://ssrg-vt.github.io/hermitux/) on [uHyve](https://github.com/hermitcore/uhyve);
 * [Lupine](https://github.com/hckuo/Lupine-Linux) on [Firecracker](https://firecracker-microvm.github.io/);
 * Lupine on KVM;
 * Linux on Firecracker;
 * Linux on KVM;
 * Linux as a userspace binary;
 * [OSv](https://github.com/cloudius-systems/osv) on KVM;
 * [Rumprun](https://github.com/rumpkernel/rumprun) on KVM;
 * Docker; and,
 * Unikraft on KVM.

We also compare [MirageOS](https://mirage.io) on Solo5, however, it does support
running NGINX as it is a Domain-Specific Language unikernel library Operating
System.  Instead, we use their [template TCP HTTP server](https://github.com/mirage/mirage-skeleton/tree/master/applications/static_website_tls)
capable of serving static content over HTTP and measure this with the same tools
and payload.

We evaluate the performance with [`wrk`](https://github.com/wg/wrk) for 1 minute
using 14 threads, 30 connections, and a static 612B HTML page.

## Usage

* `./genimages.sh` downloads and builds the tested images and takes about 4
   minutes on average;
 * `./benchmark.sh` runs the experiment and takes about 40-45 minutes on
   average; and,
 * `./plot.py` is used to generate the figure.
