# Unikraft EuroSys'21 Artifacts

This repository contains the artifacts, including experiments and graphs, for
the paper: "Building Fast, Specialized Unikernels the Easy Way"

## Experiments

The paper lists 22 figures and 3 tables worth of experiments, which both
demonstrates [Unikraft](http://unikraft.org)'s capabilities against other
existing, similar software runtime framworks as well as demonstrating the
exstensibility of Unikraft itself.  Each experiment and the relevant scripts to
generate the data and subsequent plot are included in this repository.

Each figure, table and corresponding experiment are listed below:

| Figure                                                              | Description                                                                                                                                                                                                                   | 
|---------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`fig_01`](/experiments/fig_01_linux-deps/README.md)                | Linux kernel dependency graph.                                                                                                                                                                                                |
| [`fig_02`](/experiments/fig_02_unikraft-nginx-deps/README.md)       | NGINX Unikraft dependency graph.                                                                                                                                                                                              |
| [`fig_03`](/experiments/fig_03_unikraft-helloworld-deps/README.md)  | "Hello World" Unikraft dependency graph.                                                                                                                                                                                      |
| [`fig_05`](/experiments/fig_05_syscall-heatmap/README.md)           | Syscalls required by a set of 30 popular server applications versus syscalls currently supported by Unikraft.                                                                                                                 |
| [`fig_07`](/experiments/fig_07_syscall-support/README.md)           | Syscall support for [top 30 server apps](https://popcon.debian.org/). All apps are close to being supported, and several already work even if some syscalls are stubbed (SQLite, NGINX).                                      |
| [`fig_08`](/experiments/fig_08_unikraft-image-size/README.md)       | Image sizes of Unikraft applications.  We include  permutations with and without LTO and DCE.                                                                                                                                 |
| [`fig_09`](/experiments/fig_09_compare-image-size/README.md)        | Image sizes for representative applications with Unikraft and other OSes, stripped, without LTO and DCE.                                                                                                                      |
| [`fig_10`](/experiments/fig_10_unikraft-boot/README.md)             | Boot time for Unikraft images with different virtual machine monitors.                                                                                                                                                        |
| [`fig_11`](/experiments/fig_11_compare-min-mem/README.md)           | Minimum memory needed to run different applications using different OSes, including Unikraft.                                                                                                                                 |
| [`fig_12`](/experiments/fig_12_redis-perf/README.md)                | Redis performance tested with the [`redis-benchmark`](https://github.com/redis/redis/blob/2.2/src/redis-benchmark.c), (30 connections, 100k requests, pipelining level of 16).                                                |
| [`fig_13`](/experiments/fig_13_nginx-perf/README.md)                | NGINX (and Mirage HTTP-reply) performance tested with [`wrk`](https://github.com/wg/wrk) (1 minute, 14 threads, 30 conns, static 612B HTML page).                                                                             |
| [`fig_14`](/experiments/fig_14_unikraft-nginx-alloc-boot/README.md) | Unikraft Boot time for NGINX with different allocators.                                                                                                                                                                       |
| [`fig_15`](/experiments/fig_15_unikraft-nginx-throughput/README.md) | NGINX throughput with different allocators.                                                                                                                                                                                   |
| [`fig_16`](/experiments/fig_16_unikraft-sqlite-alloc/README.md)     | Execution speedup in SQLite Unikraft, relative to [mimalloc](https://github.com/microsoft/mimalloc).                                                                                                                          |
| [`fig_17`](/experiments/fig_17_unikraft-sqlite-libc/README.md)      | Time for 60k SQLite insertions with native Linux, [newlib](https://sourceware.org/newlib/) and [musl](https://www.musl-libc.org/) on Unikraft (marked as native) and SQLite ported automatically to Unikraft (musl external). |
| [`fig_18`](/experiments/fig_18_unikraft-redis-alloc/README.md)      | Throughput for Redis Unikraft, with varying allocators and request type (`redis-benchmark`, 30 concurrent conns, 100k requests, and a pipelining level of 16.)                                                                |
| [`fig_19`](/experiments/fig_19_compare-dpdk/README.md)              | TX throughput comparison of Unikraft versus [DPDK](https://www.dpdk.org/) in a Linux VM.                                                                                                                                      |
| [`fig_20`](/experiments/fig_20_compare-9pfs/README.md)              | [9pfs](https://xenbits.xen.org/docs/unstable/misc/9pfs.html) latency for read and write operations, compared to Linux.                                                                                                        |
| [`fig_21`](/experiments/fig_21_unikraft-boot-pages/README.md)       | Unikraft boot times with static and dynamic initialization of page tables.                                                                                                                                                    |
| [`fig_22`](/experiments/fig_22_compare-vfs/README.md)               | Filesystem specialization and removal of the vfs layer yields important performance gains for a web cache when performing a look up and file open operation.                                                                  |

| Table                                                        | Experiment                                                                                                                                                                                                                           |
|--------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`tab_01`](/experiments/tab_01_bincompat-syscalls/README.md) | Cost of binary compatibility/syscalls with and without security mitigations.                                                                                                                                                         |
| [`tab_02`](/experiments/tab_02_abi/README.md)                | Results from automated porting based on externally-built archives when linked against Unikraft using musl and newlib. We show whether the port succeeded with the glibc compatibility layer ("compat layer") and without it ("std"). |
| [`tab_03`](/experiments/tab_03_kvs_compare/README.md)        | kvs_compare                                                                                                                                                                                                                          |


## Prerequisites

Before you can run these experiments, you will need to prepare your host
environment.  In the paper, we ran three nodes with Intel X running Y.
Additional configuration was ...

## Getting Started

1. To start running these experiments, begin by cloning this repository:
   ```bash
   git clone https://github.com/unikraft/eurosys21-artifacts.git
   ```

2. Many of the experiments use [Docker](#) as an intermediate tool for creating
   build and test environments (along with testing Docker itself).  Please
	 install Docker on your system to continue.

## Running experiments

We have wrapped all the individual experiments with the `run.sh` tool:

```
./run.sh - Run all or a specific experiment.

Usage:
  ./run.sh [OPTIONS] [FIGURE_ID|TEST_NAME] [prepare|run|plot]

If no figure ID or test name is provided, all tests are run.

Example:
  ./run.sh fig_01

Arguments:
  prepare         Prepares the host and/or builds dependent tools
                    and images before the test is run.
  run             Runs the given experiment and saves the results.
  plot            Uses the data from the experiment to generate
                    the plot.

Options:
  -l --list       List all tests and exit.
  -v --verbose    Be verbose.
  -h --help       Show this help menu.
```

Each experiment, and therefore directory listed in `experiments/`, is populated
with a `README.md` which includes more detail about the individual experiment.

<!--

# System requirements

```
apt install -y  curl python3 python3-click python3-tabulate \
		git python3-numpy python3-matplotlib musl-tools \
		texlive-fonts-recommended texlive-fonts-extra \
		dvipng qemu-system-x86 redis-utils socat \
		uuid-runtime bridge-utils net-tools gawk \
		qemu-utils
```

Experiments which require host precision (and therefore `ukbench`):

 * `14_redis-perf`;
 * `15_nginx-perf`;
-->
