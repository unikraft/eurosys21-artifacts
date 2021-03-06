# Unikraft Nginx TX with varying allocators

<img align="right" src="../../plots/fig_15_unikraft-nginx-throughput.svg" width="300" />

This experiment provides data for Fig. 15. Similary to Fig. 13, we
evaluate the performance of NGINX with wrk (1 minute, 14 threads, 30
conns, static 612B HTML page). In this experiment, we evaluate
Unikraft's ability to vary memory allocators.

## Usage

Run instructions:

```
cd experiments/15_unikraft-nginx-throughput
./genimages.sh
./benchmark.sh
./plot.py
```

- `./genimages.sh` takes about 5 minutes in average.
- `./benchmark.sh` takes about 20-25 minutes in average.
