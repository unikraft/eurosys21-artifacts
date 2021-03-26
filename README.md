<h1 align="center">Unikraft EuroSys'21 Artifacts</h1>

<p align="center">
  This repository contains the artifacts, including experiments and graphs, for
  the paper:
</p>

<h3 align="center">
  Unikraft: Fast, Specialized Unikernels the Easy Way
</h3>

 > **Abstract**:  Unikernels are famous for providing excellent performance in
 > terms of boot times, throughput and memory consumption, to name a few
 > metrics.  However, they are infamous for making it hard and extremely time
 > consuming to extract such performance, and for needing significant
 > engineering effort in order to port applications to them.  We introduce
 > Unikraft, a novel micro-library OS that (1) fully modularizes OS primitives
 > so that it is easy to customize the unikernel and include only relevant
 > components and (2) exposes a set of composable, performance-oriented APIs in
 > order to make it easy for developers to obtain high performance.
 > 
 > Our evaluation using off-the-shelf popular applications such as NGINX,
 > SQLite, and Redis shows that running such applications on Unikraft results in
 > a 1.7x-2.7x performance improvement compared to Linux guests.  In addition,
 > Unikraft images for these apps are around 1MB, require less than 10MB of RAM
 > to run, and boot in around 1ms on top of the VMM time (total boot time
 > 3ms-40ms). Unikraft is a Linux Foundation open source project and can be
 > found at www.unikraft.org.

[Unikraft](http://unikraft.org) is a Linux Foundation open source project and
Xen Incubator Project.  Most of the code used for this paper is upstream, or is
in the process of being upstreamed.

```
@inproceedings{Kuenzer2021,
  title = {Unikraft: Fast, Specialized Unikernels the Easy Way},
  author = {Kuenzer, Simon and Bădoiu, Vlad-Andrei and Lefeuvre, Hugo and Santhanam, Sharan and Jung, Alexander \
            and Gain, Gaulthier and Soldani, Cyril and Lupu, Costin and Teodorescu, Ștefan and Răducanu, Costi \
            and Banu, Cristian and Mathy, Laurent and Deaconescu, Răzvan and Raiciu, Costin and Huici, Felipe},
  journal = {Sixteenth European Conference on Computer Systems},
  year = {2021},
  series = {{EuroSys}{\textquotesingle}21},
  publisher = {ACM},
  address = {New York, NY, USA},
  doi = {10.1145/3447786.3456248},
  isbn = {978-1-4503-8334-9/21/04}
}
```


If at all possible, please read through this entire document before installing
or running experiments.

## Contents of this Document

  1. [Experiments](#1-experiments): List of all experiments in the paper. 
  2. [Repository structure](#2-repository-structure): Details the organization
     of this repository.
  3. [Prerequisites](#3-prerequisites): Explains how host machines were set up
     for the experiments.  
      3.1. [Hardware](#31-hardware): Details of hardware setup in the paper and
           general recommendations.  
      3.2. [Kernel](#32-kernel-and-parameters): Details on kernel requirements for the
           experiments which must be addressed manually.
  4. [Getting Started](#4-getting-started): Information on how to use this
     repository to fully recreate the experiments.  
     4.1. [`run.sh` Usage](#41-runsh-usage): Provides the help page for the
          `run.sh` wrapper program.
  5. [Notes](#5-notes): Additional notes.
  6. [Beyond the Paper](#6-beyond-the-paper): Run Unikraft in production!

## 1. Experiments

The paper comes with 22 figures, 4 tables and 2 in-line text worth of
experiments (although not all of them have experimental results, e.g., Figure 4
is an architecture diagram). Each experiment and the relevant scripts to
generate the data and subsequent plots are included in this repository.  We
expect the results generated from this artifact to match one-to-one with the
results in the paper, given that we used this artifact/scripts to actually
generate all figures in the paper.

Each figure, table and corresponding experiment are listed below:

| Figure                                                     |                                                                      | Description                                                                                                                                                                                                                   | Est. runtime |
|------------------------------------------------------------|----------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------|
| [`fig_01`](/experiments/fig_01_linux-deps/)                | <img src="plots/fig_01_linux-deps.svg" width="200" />                | Linux kernel dependency graph.                                                                                                                                                                                                | 0h 50m       |
| [`fig_02`](/experiments/fig_02_unikraft-nginx-deps/)       | <img src="plots/fig_02_unikraft-nginx-deps.svg" width="200" />       | NGINX Unikraft dependency graph.                                                                                                                                                                                              | 0h 5m        |
| [`fig_03`](/experiments/fig_03_unikraft-helloworld-deps/)  | <img src="plots/fig_03_unikraft-helloworld-deps.svg" width="200" />  | "Hello World" Unikraft dependency graph.                                                                                                                                                                                      | 0h 1m        |
| [`fig_05`](/experiments/fig_05_syscall-heatmap/)           | <img src="plots/fig_05_syscall-heatmap.svg" width="200" />           | Syscalls required by a set of [30 popular server applications](https://popcon.debian.org/) versus syscalls currently supported by Unikraft.                                                                                   | 0h 45m       |
| [`fig_07`](/experiments/fig_07_syscall-support/)           | <img src="plots/fig_07_syscall-support.svg" width="200" />           | Syscall support for [top 30 server apps](https://popcon.debian.org/). All apps are close to being supported, and several already work even if some syscalls are stubbed (SQLite, NGINX).                                      | 0h 45m       |
| [`fig_08`](/experiments/fig_08_unikraft-image-size/)       | <img src="plots/fig_08_unikraft-image-size.svg" width="200" />       | Image sizes of Unikraft applications.  We include  permutations with and without LTO and DCE.                                                                                                                                 | 0h 1m        |
| [`fig_09`](/experiments/fig_09_compare-image-size/)        | <img src="plots/fig_09_compare-image-size.svg" width="200" />        | Image sizes for representative applications with Unikraft and other OSes, stripped, without LTO and DCE.                                                                                                                      | 0h 5m        |
| [`fig_10`](/experiments/fig_10_unikraft-boot/)             | <img src="plots/fig_10_unikraft-boot.svg" width="200" />             | Boot time for Unikraft images with different virtual machine monitors.                                                                                                                                                        | 0h 9m        |
| [`fig_11`](/experiments/fig_11_compare-min-mem/)           | <img src="plots/fig_11_compare-min-mem.svg" width="200" />           | Minimum memory needed to run different applications using different OSes, including Unikraft.                                                                                                                                 | 0h 50m       |
| [`fig_12`](/experiments/fig_12_redis-perf/)                | <img src="plots/fig_12_redis-perf.svg" width="200" />                | Redis performance tested with the [`redis-benchmark`](https://github.com/redis/redis/blob/2.2/src/redis-benchmark.c), (30 connections, 100k requests, pipelining level of 16).                                                | 0h 9m        |
| [`fig_13`](/experiments/fig_13_nginx-perf/)                | <img src="plots/fig_13_nginx-perf.svg" width="200" />                | NGINX (and Mirage HTTP-reply) performance tested with [`wrk`](https://github.com/wg/wrk) (1 minute, 14 threads, 30 conns, static 612B HTML page).                                                                             | 0h 50m       |
| [`fig_14`](/experiments/fig_14_unikraft-nginx-alloc-boot/) | <img src="plots/fig_14_unikraft-nginx-alloc-boot.svg" width="200" /> | Unikraft Boot time for NGINX with different memory allocators.                                                                                                                                                                | 0h 8m        |
| [`fig_15`](/experiments/fig_15_unikraft-nginx-throughput/) | <img src="plots/fig_15_unikraft-nginx-throughput.svg" width="200" /> | NGINX throughput with different memory allocators.                                                                                                                                                                            | 0h 30m       |
| [`fig_16`](/experiments/fig_16_unikraft-sqlite-alloc/)     | <img src="plots/fig_16_unikraft-sqlite-alloc.svg" width="200" />     | Execution speedup in SQLite Unikraft, relative to [mimalloc](https://github.com/microsoft/mimalloc).                                                                                                                          | 0h 21m       |
| [`fig_17`](/experiments/fig_17_unikraft-sqlite-libc/)      | <img src="plots/fig_17_unikraft-sqlite-libc.svg" width="200" />      | Time for 60k SQLite insertions with native Linux, [newlib](https://sourceware.org/newlib/) and [musl](https://www.musl-libc.org/) on Unikraft (marked as native) and SQLite ported automatically to Unikraft (musl external). | 0h 6m        |
| [`fig_18`](/experiments/fig_18_unikraft-redis-alloc/)      | <img src="plots/fig_18_unikraft-redis-alloc.svg" width="200" />      | Throughput for Redis Unikraft, with varying memory allocators and request type (`redis-benchmark`, 30 concurrent conns, 100k requests, and a pipelining level of 16.)                                                         | 0h 5m        |
| [`fig_19`](/experiments/fig_19_compare-dpdk/)              | <img src="plots/fig_19_compare-dpdk.svg" width="200" />              | TX throughput comparison of Unikraft versus [DPDK](https://www.dpdk.org/) in a Linux VM.                                                                                                                                      | 0h 30        |
| [`fig_20`](/experiments/fig_20_compare-9pfs/)              | <img src="plots/fig_20_compare-9pfs.svg" width="200" />              | [9pfs](https://xenbits.xen.org/docs/unstable/misc/9pfs.html) latency for read and write operations, compared to Linux.                                                                                                        | 2h 0m        |
| [`fig_21`](/experiments/fig_21_unikraft-boot-pages/)       | <img src="plots/fig_21_unikraft-boot-pages.svg" width="200" />       | Unikraft boot times with static and dynamic initialization of page tables.                                                                                                                                                    | 0h 3m        |
| [`fig_22`](/experiments/fig_22_compare-vfs/)               | <img src="plots/fig_22_compare-vfs.svg" width="200" />               | Filesystem specialization and removal of the vfs layer yields important performance gains for a web cache when performing a look up and file open operation.                                                                  | 0h 5m        |

| Table                                               |                                                               | Experiment                                                                                                                                                                                                                           | Est. runtime |
|-----------------------------------------------------|---------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------|
| [`tab_01`](/experiments/tab_01_bincompat-syscalls/) | <img src="plots/tab_01_bincompat-syscalls.svg" width="200" /> | Cost of binary compatibility/syscalls with and without security mitigations.                                                                                                                                                         | 0h 25m       |
| [`tab_02`](/experiments/tab_02_abi/)                | <img src="plots/tab_02_abi.svg" width="200" />                | Results from automated porting based on externally-built archives when linked against Unikraft using musl and newlib. We show whether the port succeeded with the glibc compatibility layer ("compat layer") and without it ("std"). | 2h 0m        |
| [`tab_04`](/experiments/tab_04_kvs_compare/)        | <img src="plots/tab_04_kvs_compare.svg" width="200" />        | Performance of a specialized UDP-based in-memory key-value store on Unikraft vs. Linux.                                                                                                                                              | 1h 20m             |

| Text                                                  | Experiment                        | Est. runtime |
|-------------------------------------------------------|-----------------------------------|--------------|
| [`txt_01`](/experiments/txt_01_unikernel-boot-times/) | Unikernel boot time baseline.     | 0h 21m       |
| [`txt_02`](/experiments/txt_02_9pfs-boot-times/)      | Measures 9pfs boot time overhead. | 0h 5m        |

## 2. Repository structure

We have organised this repository as follows:

 * `experiments/` - All experiments are listed in this directory.  Each
   sub-directory is named with the figure number along with a short description
   of the experiment (e.g., `fig_01_linux-deps`).  In addition, each experiment
   sub-directory has a corresponding `README.md` which explains in more detail
   how the experiment works and how to run it.  Along with this, each
   sub-directory also comes with a `Makefile` with the following targets:
    - `prepare`: prepares the experiment, by usually downloading and building
      relevant images, tools, and auxiliary services necessary for running the
      experiment.
    - `run`: runs the experiment.
    - `plot`: produces the figure or table.  All plots are automatically saved
      into the [`plots/`](/plots) directory.
    - `clean`: removes intermediate build files.

 * `build/` - Intermediate build artifacts necessary for experiments to run.  
   This will be populated by the experiment's preparation (`prepare`) step.
 * `support/` - contains definitions of container images, `Dockerfile`s,
   which are used by multiple experiments.
 * `tools/` - contain programs which are used by multiple experiments.
 * `run.sh` - A tool that wraps all experiments and can be used to run them all
   or individually.  More details on this utility script are detailed below.

## 3. Prerequisites

### 3.1. Hardware

Before you can run these experiments, you will need to prepare 3 physical host
environments.  **Physical hosts as opposed to virtual machines are recommended
as they provide better performance.**  In the paper, we used three different
setups:

 1. A Linux host (Debian Buster) with KVM enabled and Linux kernel 4.19. This
	host is used for most experiments. We use the 4.19 kernel because HermiTux
	will not run with newer versions, as noted
	[here](https://github.com/ssrg-vt/hermitux/issues/12).
 2. A Linux host (Debian Buster) with Linux kernel 4.19 that has an 10gbit/s
	Ethernet cable connected to the first host. We use it for the DPDK network
	experiment [`fig_19`](/experiments/fig_19_compare-dpdk/) and experiments
	where we need to specifically setup the CPU frequency.
	See 3.2 for further details.
 3. A Xen host (Debian Buster) used for Xen 9pfs experiments
	([`txt_02`](/experiments/txt_02_9pfs-boot-times/)).

A single server can be used for almost all experiments, though it would require
different Linux kernel parameters, or the Xen hypervisor and rebooting to switch
from one set up to another.  The exception is the DPDK experiment, which
requires two servers connected to each other via a 10Gb link.

All of our results were run on inexpensive (roughly EUR 800)
[Shuttle SH370R6](http://global.shuttle.com/products/productsDetail?productId=2265)
boxes with an Intel i7 9700K 3.6 GHz (4.9 Ghz with Turbo Boost, 8 cores) and
32GB of RAM. For the DPDK experiment we used [Intel X520-T2](https://ark.intel.com/content/www/de/de/ark/products/69655/intel-ethernet-converged-network-adapter-x520-t2.html)
cards with the 82599EB chipset.


### 3.2. Kernel and Parameters

All experiments were run on a physical host with Debian Buster and Linux 4.19
installed. All install and preparation scripts in this repository target this
distribution and kernel version.

For all set ups, we disabled Hyper-Threading (`noht`), isolated 4 CPU cores
(e.g. `isocpus=2-6`), switched off the IOMMU (`intel_iommu=off`), and disabled
IPv6 (`ipv6.disable=1`). This can be done by setting kernel boot parameters with
your bootloader, for instance with Grub (`/etc/default/grub`):

``` bash
GRUB_CMDLINE_LINUX_DEFAULT="isolcpus=2-4 noht intel_iommu=off ipv6.disable=1"
```

or with syslinux/pxelinux:

``` text
...
LABEL item_kernel0
  MENU LABEL Linux
  MENU DEFAULT
  KERNEL vmlinuz-4.19.0
  APPEND isolcpus=2-6 noht intel_iommu=off ipv6.disable=1
  ...
```

On Xen we use the following parameters (please adjust the amount of pinned
memory for Dom0 according to your available RAM, we gave the half of 32GB RAM to
Dom0; We also pinned 4 CPU cores to Dom0):
Grub (`/etc/default/grub`):

``` bash
GRUB_CMDLINE_LINUX_XEN_REPLACE_DEFAULT=""
GRUB_CMDLINE_LINUX_XEN_REPLACE="earlyprintk=xen console=hvc0 ipv6.disable=1"

# Xen boot parameters for all Xen boots
GRUB_CMDLINE_XEN=""
# Xen boot parameters for non-recovery Xen boots (in addition to GRUB_CMDLINE_XEN)
GRUB_CMDLINE_XEN_DEFAULT="dom0_vcpus_pin dom0_max_vcpus=4 smt=0 dom0_mem=15360M,max:16384M cpufreq=xen gnttab_max_frames=256"
```

Please note that the following experiments require additional kernel parameters
e.g., to enable specific CPU frequency scaling governors:
- [`tab_01`](/experiments/tab_01_bincompat-syscalls/)
- [`tab_04`](/experiments/tab_04_kvs_compare/)
- [`fig_19`](/experiments/fig_22_compare-dpdk/)
- [`fig_22`](/experiments/fig_22_compare-vfs/)

We documented this within the experiment folder.

## 4. Getting Started

1. Before running any of these experiments, prepare your host with the
   recommendations detailed above in [prerequisites](#3-prerequisites);

2. Many of the experiments use Docker as an intermediate tool for creating build
   and test environments (along with testing Docker itself).  Please
   [install Docker](https://docs.docker.com/get-docker/) on your system to
   continue;

3. Once Docker is installed, clone this repository
   ```bash
   git clone https://github.com/unikraft/eurosys21-artifacts.git
   ```

4. All experiments should be `prepare`d first, which installs necessary tools
   and downloads additional resources, before they can run.  This can be done by
   calling `run.sh fig_XX prepare` ([more details below](#41-runsh-usage)) for a
   single experiment or `run.sh prepare` for all experiments.  (Note: The
   preparation step for all experiments usually exceeds several hours.)

5. Once prepared, simply call the relevant experiment you wish to re-create
   using the `run.sh` script.

Please note that we recommend to use
[qemu-system-x86](https://packages.debian.org/buster-backports/qemu-system-x86)
from the official Debian Buster repositories, version `1:3.1+dfsg-8+deb10u8`.
Note that Rumprun experiments fail with the version from buster-backports
(`1:5.2+dfsg-3~bpo10+1`), possibly due to a bug either in Rumprun or in the
Debian package.

### 4.1. `run.sh` Usage

We have wrapped all the individual experiments with the `run.sh` tool.  This
script will install the necessary dependencies for all experiments (excluding
Docker) only for Debian Buster.  Please see [prerequisites](#3-prerequisites)
and [getting started](#4-getting-started) guide above.

```
./run.sh - Run all or a specific experiment.

Usage:
  ./run.sh [OPTIONS] [FIGURE_ID|TEST_NAME] [ACTION]

If no figure ID or test name is provided, the action is run for all
experiments.  If no action is provided, all actions are performed.

Examples:
  ./run.sh fig_01      Runs prepare, run and plot for fig_01.
  ./run.sh prepare     Prepares all experiments.

Actions:
  prepare              Prepares the host and/or builds dependent tools
                         and images before the test is run.
  run                  Runs the given experiment and saves the results.
  plot                 Uses the data from the experiment to generate
                         the plot.
  clean                Clean intermediate build files from an experiment.

Options:
     --no-deps         Do not try to install dependencies.
     --no-docker-plot  Do not use Docker for plotting.
  -l --list            List all tests and exit.
  -v --verbose         Be verbose.
  -h --help            Show this help menu.

Influential Environmental Variables
  EXPERIMENTS_DIR      Directory of all the experiments
                         (default: ./experiments).
  DOCKER_FORCE_BUILD   Force build of Docker containers
                         (default: y).
  DOCKER_IMAGE_PLOT    Docker environment for generating plots
                         (default: unikraft/eurosys21-artifacts-plot:latest).
  PLOT_FORMAT          File format for the plot
                         (default: svg).
```

Each experiment, and more specifically its sub-directory in `experiments/`, is
populated with a `README.md` which includes more detail about the individual
experiment.

## 5. Notes

 * All experiments should be run as the `root` user on the host as it will
   require modifications to the host and running with elevated privileges.

 * We use intermediate Docker containers for building images and accessing
   pre-built binaries for many of the experiments.  In addition to this, this
   repository clones the Linux kernel to make changes for testing.  As a result,
   expected disk storage utilized to conduct all experiments is ~50GB.

 * The preparation step for all experiments usually exceeds several hours.

 * Experiments cannot be run in parallel due to overlapping CPU core affinities,
   which will affect measurements.

 * While each experiment has its own sub-directory and a `Makefile` script
   within it, we strongly suggest to use the main `run.sh` script that wraps all
   experiments (more on this in Section 4.1 above).

 * Some experiments (e.g., [`fig_22`](/experiments/fig_22_compare-vfs/)) produce
   some error messages but still finish and correctly produce the plot; if this
   is the case, this is documented in an experiment's sub-directory, in its own
   `README.md` file.

 * All plots are saved into the global [`plots/`](/plots) directory when run via
   `run.sh`.  When using the individual experiment's `Makefile`, it is saved to
   the experiment's folder.

## 6. Beyond the Paper

This repository only contains the performance evaluation of Unikraft.
In addition to this README, the Unikraft project provides [extensive
documentation](http://docs.unikraft.org/) on how to use Unikraft in
real-world environments. In addition, interested researchers are
welcome to join the community via the Xen project [mailing
list](https://lists.xenproject.org/cgi-bin/mailman/listinfo/minios-devel)
and [GitHub](https://github.com/unikraft).
