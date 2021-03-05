# Unikraft Nginx TX with varying allocators

This experiment provides data for Fig. 15. The performance of NGINX is
evaluated with wrk (1 minute, 14 threads, 30 conns, static 612B HTML
page).

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
