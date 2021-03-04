# Unikraft EuroSys'21 Artifacts

This repository contains the artifacts, including experiments and graphs, for
the paper: "Building Fast, Specialized Unikernels the Easy Way"

## Experiments

The paper lists 22 figures and 3 tables worth of experiments, which both
demonstrate [Unikraft](http://unikraft.org)'s capabilities against other
existing, similar frameworks as well as demonstrating the
exstensibility of Unikraft itself.  Each experiment and the relevant scripts to
generate the data and subsequent plot are included in this repository.

Each figure, table and corresponding experiment are listed below:

| Figure                                                              |       | Description                                                                                                                                                                                                                   | Est. runtime |
|---------------------------------------------------------------------|-------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------|
| [`fig_01`](/experiments/fig_01_linux-deps/README.md)                |       | Linux kernel dependency graph.                                                                                                                                                                                                | 0h 17m       |
| [`fig_02`](/experiments/fig_02_unikraft-nginx-deps/README.md)       |       | NGINX Unikraft dependency graph.                                                                                                                                                                                              | 0h 5m        |
| [`fig_03`](/experiments/fig_03_unikraft-helloworld-deps/README.md)  |       | "Hello World" Unikraft dependency graph.                                                                                                                                                                                      | 0h 1m        |
| [`fig_05`](/experiments/fig_05_syscall-heatmap/README.md)           |       | Syscalls required by a set of 30 popular server applications versus syscalls currently supported by Unikraft.                                                                                                                 |              |
| [`fig_07`](/experiments/fig_07_syscall-support/README.md)           |       | Syscall support for [top 30 server apps](https://popcon.debian.org/). All apps are close to being supported, and several already work even if some syscalls are stubbed (SQLite, NGINX).                                      |              |
| [`fig_08`](/experiments/fig_08_unikraft-image-size/README.md)       |       | Image sizes of Unikraft applications.  We include  permutations with and without LTO and DCE.                                                                                                                                 | 1h 10m       |
| [`fig_09`](/experiments/fig_09_compare-image-size/README.md)        |       | Image sizes for representative applications with Unikraft and other OSes, stripped, without LTO and DCE.                                                                                                                      | 0h 5m        |
| [`fig_10`](/experiments/fig_10_unikraft-boot/README.md)             |       | Boot time for Unikraft images with different virtual machine monitors.                                                                                                                                                        | 0h 7m        |
| [`fig_11`](/experiments/fig_11_compare-min-mem/README.md)           |       | Minimum memory needed to run different applications using different OSes, including Unikraft.                                                                                                                                 |              |
| [`fig_12`](/experiments/fig_12_redis-perf/README.md)                |       | Redis performance tested with the [`redis-benchmark`](https://github.com/redis/redis/blob/2.2/src/redis-benchmark.c), (30 connections, 100k requests, pipelining level of 16).                                                | 0h 6m        |
| [`fig_13`](/experiments/fig_13_nginx-perf/README.md)                |       | NGINX (and Mirage HTTP-reply) performance tested with [`wrk`](https://github.com/wg/wrk) (1 minute, 14 threads, 30 conns, static 612B HTML page).                                                                             | 0h 45m       |
| [`fig_14`](/experiments/fig_14_unikraft-nginx-alloc-boot/README.md) |       | Unikraft Boot time for NGINX with different allocators.                                                                                                                                                                       | 0h 2m        |
| [`fig_15`](/experiments/fig_15_unikraft-nginx-throughput/README.md) |       | NGINX throughput with different allocators.                                                                                                                                                                                   | 0h 25m       |
| [`fig_16`](/experiments/fig_16_unikraft-sqlite-alloc/README.md)     |       | Execution speedup in SQLite Unikraft, relative to [mimalloc](https://github.com/microsoft/mimalloc).                                                                                                                          | 0h 16m       |
| [`fig_17`](/experiments/fig_17_unikraft-sqlite-libc/README.md)      |       | Time for 60k SQLite insertions with native Linux, [newlib](https://sourceware.org/newlib/) and [musl](https://www.musl-libc.org/) on Unikraft (marked as native) and SQLite ported automatically to Unikraft (musl external). |              |
| [`fig_18`](/experiments/fig_18_unikraft-redis-alloc/README.md)      |       | Throughput for Redis Unikraft, with varying allocators and request type (`redis-benchmark`, 30 concurrent conns, 100k requests, and a pipelining level of 16.)                                                                | 0h 3m        |
| [`fig_19`](/experiments/fig_19_compare-dpdk/README.md)              |       | TX throughput comparison of Unikraft versus [DPDK](https://www.dpdk.org/) in a Linux VM.                                                                                                                                      |              |
| [`fig_20`](/experiments/fig_20_compare-9pfs/README.md)              |       | [9pfs](https://xenbits.xen.org/docs/unstable/misc/9pfs.html) latency for read and write operations, compared to Linux.                                                                                                        |              |
| [`fig_21`](/experiments/fig_21_unikraft-boot-pages/README.md)       |       | Unikraft boot times with static and dynamic initialization of page tables.                                                                                                                                                    | 0h 1m        |
| [`fig_22`](/experiments/fig_22_compare-vfs/README.md)               |       | Filesystem specialization and removal of the vfs layer yields important performance gains for a web cache when performing a look up and file open operation.                                                                  |              |

| Table                                                        | Experiment                                                                                                                                                                                                                           | Est. runtime |
|--------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------|
| [`tab_01`](/experiments/tab_01_bincompat-syscalls/README.md) | Cost of binary compatibility/syscalls with and without security mitigations.                                                                                                                                                         |              |
| [`tab_02`](/experiments/tab_02_abi/README.md)                | Results from automated porting based on externally-built archives when linked against Unikraft using musl and newlib. We show whether the port succeeded with the glibc compatibility layer ("compat layer") and without it ("std"). |              |
| [`tab_03`](/experiments/tab_03_kvs_compare/README.md)        | kvs_compare                                                                                                                                                                                                                          |              |


## Prerequisites

### Hardware

Before you can run these experiments, you will need to prepare your host
environment.  In the paper, we used three different set ups:

 1. A Linux host with KVM enabled and Linux kernel 4.19 (most
    experiments). We use a somewhat older kernel because HermiTux will
    not run with newer versions, as noted [here](https://github.com/ssrg-vt/hermitux/issues/12).
 2. A Linux host with a recent Linux kernel (5.11) used as a DPDK packet
    generator ([`fig_19`](/experiments/fig_19_compare-dpdk/README.md)).  In
    addition, we patch the kernel with [these patches](#) to conduct syscall
    overhead experiments ([`tab_01`](/experiments/tab_01_bincompat-syscalls/README.md)).
 3. A Xen host used for Xen 9pfs experiments.). 

A single server can be used for almost all experiments, though it would require
installing different Linux kernel versions, or the Xen hypervisor and rebooting
to switch from one set up to another.  The exception is the DPDK experiment,
which requires two servers connected to each other via a 10Gb link.

All of our results were run on inexpensive (roughly EUR 800)
[Shuttle SH370R6](http://global.shuttle.com/products/productsDetail?productId=2265)
boxes with an Intel i7 9700K 3.6 GHz (4.9 Ghz with Turbo Boost, 8 cores) and
32GB of RAM. For the DPDK experiment we used[Intel X520-T2](https://ark.intel.com/content/www/de/de/ark/products/69655/intel-ethernet-converged-network-adapter-x520-t2.html)
cards with the 82599EB chipset.

### Software

#### Kernel

All experiments were on Debian Buster

For all set ups, we disabled Hyper-Threading (`noht`) and isolated 4 CPU cores
(e.g. `isocpus=4,5,6,7`).  This can be done by setting kernel boot parameters,
e.g. with pxelinux:

```
...
LABEL item_kernel0
  MENU LABEL Linux
  MENU DEFAULT
  KERNEL vmlinuz-4.19.0
  APPEND isolcpus=4,5,6,7 noht
  ...
```

From the remaining 4 CPU cores, we pinned one to the VM, another one to the VMM
(e.g., `qemu-system-x86_64`), and another one to the client tool (e.g., `wrk` or
`redis-benchmark`).  For all experiments, we set the governor to performance,
which can be done generally by:

```
echo "performance" > /sys/devices/system/cpu/cpu$CPU_ID/cpufreq/scaling_governor
```

However, both the pinning and governor settings are handled by the scripts in
this repo (as opposed to the kernel boot parameters, which you will need to take
care of manually).

For [`fig_22`](/experiments/fig_22_compare-vfs/README.md) we made changes to
Linux kernel (TODO: Simon).

#### Xen

For 

#### DPDK

For [`fig_19`](/experiments/fig_19_compare-dpdk/README.md), we installed
[DPDK](dpdk.org) which has [additional installation
instructions](http://doc.dpdk.org/guides/linux_gsg/sys_reqs.html#compilation-of-the-dpdk).
We have provided a convience script to help with the installation process,
[`tools/install-dpdk.sh`](/tools/install-dpdk.sh).

## Getting Start

1. To start running these experiments, begin by cloning this repository:
   ```bash
   git clone https://github.com/unikraft/eurosys21-artifacts.git
   ```

2. Many of the experiments use [Docker](https://docs.docker.com/get-docker/) as
   an intermediate tool for creating build and test environments (along with
   testing Docker itself).  Please install Docker on your system to continue.

## Running experiments

We have wrapped all the individual experiments with the `run.sh` tool:

```
./run.sh - Run all or a specific experiment.

Usage:
  ./run.sh [OPTIONS] [FIGURE_ID|TEST_NAME] [ACTION]

If no figure ID or test name is provided, the action is run for all
experiments.

Example:
  ./run.sh fig_01

Actions:
  prepare            Prepares the host and/or builds dependent tools
                       and images before the test is run.
  run                Runs the given experiment and saves the results.
  plot               Uses the data from the experiment to generate
                       the plot.
  clean              Clean intermediate build files from an experiment.

Options:
  -D --no-docker     Do not use Docker for plotting.
  -l --list          List all tests and exit.
  -v --verbose       Be verbose.
  -h --help          Show this help menu.

Influential Environmental Variables
  EXPERIMENTS_DIR    Directory of all the experiments
                       (default: ./experiments).
  DOCKER_IMAGE_PLOT  Docker environment for generating plots
                       (default: unikraft/eurosys21-artifacts-plot:latest).
  PLOT_FORMAT        File format for the plot
                       (default: svg).
```

Each experiment, and therefore directory listed in `experiments/`, is populated
with a `README.md` which includes more detail about the individual experiment.

<!--

Experiments which require host precision (and therefore `ukbench`):

 * `14_redis-perf`;
 * `15_nginx-perf`;
-->
