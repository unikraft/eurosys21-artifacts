# Unikraft page table initialization boot times

<img align="right" src="../../plots/fig_21_unikraft-boot-pages.svg" width="300" />

This experiment provides data for Fig. 21.

## Usage

Run instructions:

```
cd experiments/21_unikraft-boot-pages
./genimages.sh
./benchmark.sh
 python ./plot.py --data=results --output=boot_times.png
```

- `./genimages.sh` takes two minutes in average.
- `./benchmark.sh` takes less than a minute in average.
