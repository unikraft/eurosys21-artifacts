# Boot time of Unikraft with different VMMs

This experiment provides data for Fig. 10.

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
