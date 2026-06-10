# Unikraft SQLite with varying allocators

<img align="right" src="../../plots/fig_16_unikraft-sqlite-alloc.svg" width="300" />

This experiment provides data for Fig. 16.

## Usage

Run instructions:

```
cd experiments/16_unikraft-sqlite-alloc
./genimages.sh
./benchmark.sh
python ./plot.py --data=results --output=boot_times.png
```

- `./genimages.sh` takes about 5 minutes in average.
- `./benchmark.sh` takes about 16 minutes in average.
