# Unikraft Redis TX with varying allocators

This experiment provides data for Fig. 18.

We measure the throughput (GET and SET requests) of Redis Unikraft,
with varying allocators and request type (`redis-benchmark`, 30
concurrent conns, 100k requests, and a pipelining level of 16.)

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
