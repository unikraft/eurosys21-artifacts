# NGINX Throughput baseline

This experiment provides data for Fig. 13. We evaluate the performance
of NGINX with wrk (1 minute, 14 threads, 30 conns, static 612B HTML
page).

## Usage

Run instructions:

```
cd experiments/15_nginx-perf
./genimages.sh
./benchmark.sh
```

- `./genimages.sh` takes about 4 minutes in average.
- `./benchmark.sh` takes about 40-45 minutes in average.
