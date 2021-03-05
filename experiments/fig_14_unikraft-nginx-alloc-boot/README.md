# Unikraft NGINX boot time with varying allocators

This experiment provides data for Fig. 14.

We measure the guest boot time (not including VMM overhead) and
provide a per-component breakdown to highlight the impact of memory
allocator choice on boot time. We focus on an NGINX unikernel in
QEMU/KVM (i.e., including network stack initialization, PCI-bus
enumeration, a full-fledged libc, among others) thus representing an
absolute upper bound on guest boot time.

## Usage

Run instructions:

```
cd experiments/14_unikraft-nginx-alloc-boot
./genimages.sh
./benchmark.sh
./plot.py
```

- `./genimages.sh` takes about 5 minutes in average.
- `./benchmark.sh` takes about 1.5 minutes in average.

Note: this experiment relies on [Unikraft
tracepoints](http://docs.unikraft.org/developers-debugging.html). This
requires the registration of a custom GDB script (should be set up by
`run.sh`):

```
mkdir -p unikraft-scripts
pushd unikraft-scripts
git clone https://github.com/unikraft/unikraft.git
pushd unikraft
git checkout 7fd6797bd5917acc515ef6ddbfa85621f4aacf5f
popd
popd
echo "source $(pwd)/unikraft-scripts/unikraft/support/scripts/uk-gdb.py" >> ~/.gdbinit
```
