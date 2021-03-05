# Unikernel boot time baseline

This experiment provides data for unikernel boot times (not a plot, in
the paper's text). Note that this experiment does not provide data for
Unikraft; refer to Figs. 10, 14, and 21.

## Usage

Run instructions:

```
cd experiments/txt_01_unikernel-boot-times
./genimages.sh
./benchmark.sh
```

- `./genimages` takes less than a minute in average.
- `./benchmark.sh` takes about 20 minutes in average.
