### 9pfs latency

This repository contains the experiments for the 9pfs latency. 
Since these experiments require a large VM, we decided to host the zip
file which contains the Linux VM on another server.

In order to setup the environment for these experiments, please 
execute first the `./setup.sh` script. This script will setup all the
environement to test the 9pfs latency. Once the setup is done, you
have two choices:

## Test unikraft with 9pfs:
    
1. For testing read latency, go to `apps/lib-readfile/`:
    - Run `./run_all.sh` (wait some times) 
    - Raw results will saved into `rhea-kvm_read.txt`;
    - Formatted results will be saved into `rhea-kvm_read.csv`;
2. For testing write latency, go to `apps/lib-writefile/`:
    - Run `./run_all.sh` (wait some times) 
    - Raw results will saved into `rhea-kvm_write.txt`;
    - Formatted results will be saved into `rhea-kvm_write.csv`;
    - Note that this experiment may take some time.

## Test VM Linux with 9pfs (require some manual steps):

1. Go to the `vm` folder;
2. Run the launcher script (`./launcher.sh`);
3. Enter login/pwd: `root/root` in the guest VM;
4. In the VM prompt, only run the `./script.sh` script;
    - Raw results will be saved into `linux-kvm_read.txt` and
    `linux-kvm_write.txt`;
    - Formatted results will be saved into `test/linux-kvm_read.csv`
    and `test/linux-kvm_write.csv`. They will be accessible from the
    host;
    - The guest VM will be automatically stopped at the end of the
    script;
    - Note that this experiment may take some time.

## Generate the graph

Once all the results are generated, run the `./plot.sh` to generate
the 9pfs latency plot. This one will be egenerated in the current
folder with the following name: `compare_9pfs_lines.pdf`.