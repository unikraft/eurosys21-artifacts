# Comparison of Unikernel Image Sizes

<img align="right" src="../../plots/fig_09_compare-image-size.svg" width="300" />

From the paper, "the main advantage of unikernels over traditional operating
systems is their low resource consumption."  To demonstrate this, we evaluated
the image size of 4 representative applications in Unikraft and on other
unikernel projects, including [HermiTux](https://ssrg-vt.github.io/hermitux/),
[Lupine](https://github.com/hckuo/Lupine-Linux), [MirageOS](mirage.io),
[OSv](https://github.com/cloudius-systems/osv), [Rumprun](https://github.com/rumpkernel/rumprun),
and as a stripped, dynamically-linked native Linux userspace binary.  This
figure shows that "Unikraft images are all under 2MBs for all of these
applications."

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