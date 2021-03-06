# Boot time of Unikraft with different VMMs

<img align="right" src="../../plots/fig_10_unikraft-boot.svg" width="300" />

From the paper, "the main advantage of unikernels over traditional operating
systems is their low resource consumption. ... Small image sizes are not only
useful for minimizing disk storage, but also to enable quick boot times for VMs
based on those images.  [LightVM](https://github.com/sysml/lightvm) has shown
that it is possible to boot a no-op unikernel in around 2ms, with a heavily
optimized Xen toolstack.  In our evaluation, we use standard virtualization
toolstacks instead, and wish to understand how quickly Unikraft VMs can boot.
When running experiments, we measure both the time taken by the VMM (e.g.
[Firecracker](https://firecracker-microvm.github.io/), QEMU,
[Solo5](https://github.com/Solo5/solo5/)) and the boot time of the actual
unikernel/VM, measured from when the first guest instruction is run until
`main()` is invoked.  The results are shown in [this figure], showing how long
a helloworld unikernel needs to boot with different VMMs.  Note that Unikraft's
boot time (only the guest, without VMM overheads) ranges from tens to hundreds
of microseconds when the VM has no devices, and up to 1ms when the VM has one
networking interface."

## Usage

Run instructions:

```
cd experiments/fig_10_unikraft-boot
./genimages.sh
./benchmark.sh
python3 plot.py
```

- `./genimages.sh` takes about 2m in average.
- `./benchmark.sh` takes about 6m30s in average.

Note: like Fig. 14, this experiment relies on Unikraft tracepoints.
Refer to Fig. 14's [README.md](https://github.com/unikraft/eurosys21-artifacts/blob/master/experiments/fig_14_unikraft-nginx-alloc-boot/README.md)
for instructions.
