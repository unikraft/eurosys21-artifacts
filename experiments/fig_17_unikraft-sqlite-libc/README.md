# SQLite Performance Experiment

## Goal

This folder is used for a performance experiment of SQLite running on
[Unikraft][].

[Unikraft]: https://github.com/unikraft/unikraft

This experiment compares four different configuration:

- `newlib-native` uses the native port of SQLite to Unikraft, with the
  [newlib][] libc and [pthread-embedded][].
- `musl-native` uses the native port of SQLite to Unikraft, with [musl][] libc.
- `musl-compat` uses a static library of SQLite built with `musl-gcc` on a
  standard GNU\Linux box (we recommend Debian), which is then linked with
  Unikraft, also using [musl][].
- `linux-native` does not use Unikraft at all. It just runs the same benchmark
  running SQLite built with `musl-gcc`, as a standard Linux process (no
  virtualisation).

All Unikraft experiments use the [TLSF][] memory allocator, while the Linux one
uses the default Linux allocator.

[newlib]: https://github.com/unikraft/lib-newlib
[musl]: https://github.com/unikraft/lib-musl
[pthread-embedded]: https://github.com/unikraft/lib-pthread-embedded
[TLSF]: https://github.com/unikraft/lib-tlsf

## How to Use

Running the `sqlite_experiments.sh` as a user will:

- create a temporary folder;
- download SQLite, Unikraft and the relevant Unikraft libraries;
- configure, build and run the different test applications (10 times each).

For the Linux test, if `/tmp` is on tmpfs, i.e. a RAM disk, it will be used.
Otherwise the experiment will create a RAM disk. It is important to use a RAM
disk to be fair as the Unikraft experiments are using the RAM disk approach,
and the benchmark is I/O intensive. Note that on some Linux boxes, using `/tmp`
on tmpfs is actually faster than using a dedicated RAM disk mounted using tmpfs
as well (sync operations return immediately with `/tmp` while provoking context
switches with other RAM disks).

After execution of the `sqlite_experiments.sh` script, the temporary folder
will contain the built files. You can inspect the applications and re-run their
experiments using the `run.sh` in the `apps` folders, or by calling
`sqlite_experiments.sh --run`.

Note that with no information about the system, those `run.sh` scripts assume
CPU3 to be available, and only fix the CPU frequency governor to performance
(e.g. you are still responsible for isolating that CPU, disabling turboboost,
etc.). It is up to you to make that setup if needed. Both the main
`sqlite_experiments.sh` and individual `run.sh` scripts are easy to modify
directly.
