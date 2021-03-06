# Measures 9pfs boot time overhead.

Scripts to run:
 * `prepare.sh` - prepares the environment
 * `run_kvm.sh` - runs the experiment on kvm
 * `run_xen.sh` - runs the experiment on xen
 * `compute_results.py` - computes the average and prints the result
 * `clean.sh` - Cleans up

Results available in the folder `results/`.  These are the raw result in
nanoseconds.

To print the results in human readable format
```
python3 compute_results.py
```
