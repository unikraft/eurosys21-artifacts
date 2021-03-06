# Unikraft Redis TX with varying allocators

<img align="right" src="../../plots/fig_18_unikraft-redis-alloc.svg" width="300" />

This experiment provides data for Fig. 18.

Similarly to Fig. 12, we measure the throughput of Redis Unikraft with varying
request types (`redis-benchmark`, 30 concurrent conns, 100k requests, and a
pipelining level of 16.). In this experiment we focus on Unikraft's ability to
vary allocators.

## Usage

Run instructions:

```
cd experiments/18_unikraft-redis-alloc
./genimages.sh
./benchmark.sh
./plot.py
```

- `./genimages.sh` takes about 2 minutes in average.
- `./benchmark.sh` takes about 3 minutes in average.
