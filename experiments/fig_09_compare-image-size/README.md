# Comparison of Unikernel Image Sizes

This experiment provides data for Figure 10.

## Usage

Run instructions:

```bash 
cd experiments/10_compare-image-size
./imgsize.sh
python3 plot.sh
```

`./imgsize.sh` takes about 5 minutes in average.

  N.B. You may need to set `UK_KRAFT_GITHUB_TOKEN` as a prefix to `make
  docker` in the top-level directory if you are rate-limited by kraft, e.g.:
  ```
  DOCKER_BUILD_EXTRA="--build-arg UK_KRAFT_GITHUB_TOKEN=<mytoken>" make docker-kraft
  ```