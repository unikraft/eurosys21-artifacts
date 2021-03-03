# Comparing image sizes of Unikraft applications

This experiment tests 4 configurations:

 * Default configuration;
 * with Link-Time Optimizations (LTO);
 * with Dead Code Elimination (DCE); and,
 * with DCE and LTO,

against 4 Unikraft unikernels:

 * A simple "Hello World" C program;
 * NGINX;
 * SQlite; and,
 * Redis.

These image permutations are constructed using
[kraft](https://github.com/unikraft/kraft) via a [`Dockerfile`](/Dockerfile).

  N.B. You may need to set `UK_KRAFT_GITHUB_TOKEN` as a prefix to `make docker`.

| Target             | Estimated time |
|--------------------|----------------|
| `make docker`      | 2m 24s         |
| `make helloworld`  | 2m 18s         |
| `make redis`       | 22m 48s        |
| `make sqlite`      | 20m 48s        |
| `make nginx`       | 27m 7s         |
