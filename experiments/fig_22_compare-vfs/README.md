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
   build a Unikraft unikernel for benchmarking and that contains
   SHFS and vfscore.
2. `./build-shfstools.sh` - Compile needed tools to create an SHFS
   disk image needed for the benchmark.
3. `./build-benchmark.sh` - Builds the benchmark (1) as ELF binary for
   Linux VFS benchmarks and (2) as Unikraft unikernel for SHFS and
   vfscore benchmarks. The resulting binaries will be within `src/`
   and `src/build/`.
4. `./build-fses.sh` - Creates (1) an initramdisk for Linux that only
   contains the benchmark program `fsbench` as init process and the
   testfile (`rnd4k`), (2) an initramdisk for Unikraft that contains
   only the testfile, and (3) an SHFS disk image containing only the
   testfile.
   The benchmark will measure the time that the kernels need to open a
   non-existing file and the testfile. The testfile is opened with
   its SHA256 hash digest as filename.
5. `./run.sh` - Run the experiments with KVM, console outputs are
   stored under `eval/` for later evaluation. The Linux VM experiments
   are done with the host kernel. The script takes
   `/boot/vmlinuz-$( uname -r )` as kernel.
6. `./parse.sh` - Parses the console outputs, intermediate data is
   placed under `eval/parsed/`
7. `./process.sh` - Generates CSV files containing the results ready
   for being plotted. The CSV files are placed under `results/`
8. `./plot.py` - Plots a comparison graph based on the CSV files placed
   under `results/`.

Alternatively, you can execute these steps with `make all`.
All steps together take roughly 5 mins.

With `./clean.sh` you can delete all compilation units, cloned
repositories and intermediate measurement data. It keeps the final
CSV files and plot of the experiment within `results/`.
