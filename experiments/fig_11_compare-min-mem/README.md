# Minimum memory needed for varying applications on varying frameworks

<img align="right" src="../../plots/fig_11_compare-min-mem.svg" width="300" />

From the paper, "in order to understand how many unikernel VMs we could pack on
a single server when RAM is a bottleneck, we ran experiments to measure the
minimum amount of memory required to boot various applications as unikernels,
finding that 2-6MBs of memory suffice for Unikraft guests."

This experiment provides data for Fig. 11.

## Usage

Run instructions:

 * `./genimages.sh` downloads and builds the tested images anm takes about 2m
   on average;
 * `./benchmark.sh` runs the experiment and takes about 6m30s in average; and
 * `./plot.py` is used to generate the figure.
