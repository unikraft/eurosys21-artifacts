# Redis TX baseline

This experiment provides data for Fig. 12.

We measure the throughput of Redis in a wide range of systems (unikernels, but
also Linux KVM and native), with varying request type (`redis-benchmark`, 30
concurrent conns, 100k requests, and a pipelining level of 16.)

## Usage

Run instructions:

```
cd experiments/12_redis-perf
./genimages.sh
./benchmark.sh
python3 plot.py
```

- `./genimages.sh` takes about 3 minutes in average.
- `./benchmark.sh` takes about 6 minutes in average.
