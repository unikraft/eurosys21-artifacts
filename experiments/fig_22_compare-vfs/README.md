# VFS Specialization Performance

The experiments in this folder will evaluate the performance
when specializing the VFS with Unikraft.
The same benchmark program under `src/` is compiled as (1) Unikraft
unikernel and (2) as Linux ELF binary. It will try to open for 1000
times a non-existing file and for 1000 times an existing file through
the VFS on Linux and Unikraft (`open()`). In case of Unikraft, a
second evaluation is done by using the native SHFS API
(`shfs_fio_open()`). SHFS uses a hash-table as content catalog and
was originally developed for MiniCache
(<https://github.com/cnplab/minicache>), a specialized unikernel that
can be used as a web content cache node.

Please note that these experiments do their measurement with the TSC.
You need to make sure that your machine does dynamically scale the CPU
frequency while running the experiment. This makes sure that the TSC
clock can be considered stable.
On recent Intel CPUs you can achieve pinning the CPU frequency by
adding `intel_pstate=disable` to the Linux kernel arguments of
the host (on Debian see `/etc/default/grub`). After a reboot, our
script `/tools/tunecpumax` can help you enabling the `userspace`
frequency scaling governor on your host and setting up the maximum
non-turbo frequency to your CPU.

## Usage

1. `./clone-deps.sh` - Clones needed repositories from GitHub to
   build a Unikraft unikernel that contains SHFS and vfscore.
2. `./build-shfstools.sh` - Compile needed tools to create an SHFS
   disk image needed for the benchmark.
3. `./build-benchmark.sh` - Builds the benchmark (1) as ELF binary for
   Linux VFS benchmarks and (2) as Unikraft unikernel for SHFS and
   vfscore benchmarks within `src/`.
4. `./build-fses.sh` - Creates (1) an initramdisk for Linux that only
   contains the benchmark program `fsbench` as init process and the
   testfile (rnd4k), (2) an initramdisk with only the testfile for the
   Unikraft unikernel, and (3) an SHFS volume containing only the
   testfile for the Unikraft unikernel, too.
   The benchmark will measure the time that the kernel needs to open a
   non-existing file and the testfile. The testfile is provided with
   its SHA hash over its content as filename.
5. `./run.sh` - Run the experiments, console outputs are stored under
   `eval/` for later evaluation. The Linux VM experiments are done with
   the host kernel. The script looks for `/boot/vmlinuz-$( uname -r )`.
6. `./parse.sh` - Parses the console outputs, intermediate data is
   placed under `eval/parsed/`
7. `./process.sh` - Generates CSV files containing the results ready
   for being plotted. The CSV files are placed under `results/`
8. `./plot.py` - Plots the graph based on the CSV files places under
   `results/`.

Alternatively, you can execute these steps with `make all`.
All steps together take roughly 10-20 mins.

With `./clean.sh` you can delete all compilation units, cloned
repositories and intermediate measurement data. It keeps the final
overview table of the experiment within `results/`.
