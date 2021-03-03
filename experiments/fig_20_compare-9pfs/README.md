# 9pfs latency

## Goal

This folder is used for a performance experiment of 9pfs between
[Unikraft][] and a Linux VM (write and read latency).
Since these experiments require a large VM, we decided to host the zip
file which contains the Linux VM on another server.

[Unikraft]: https://github.com/unikraft/unikraft

This experiment takes some time and requires
a very few human interaction (entering `root` as login and password
and running the `./script` when launching the Linux VM).

**APPROXIMATIVE EXECUTION TIME=2hours**

## How to Use (automatic test)

You can automatically run all the experiment by running the `make all`
command. This command will:

1. Prepare the environment by executing the `./setup.sh` script;
2. Run the unikraft and Linux VM latency experiment (respectively
`./run_unikraft.sh` and `./run_vm_linux.sh` scripts). Note that
these script take long...
3. Generate the figure by running the `./plot.sh` script and then save
it into the current repository.

## How to Use (manual test)

You can also test manually by executing the following procedure.

In order to setup the environment for these experiments, please 
execute first the `./setup.sh` script. This script will setup all the
environment to test the 9pfs latency. Once the setup is done, proceed
as follow:

### Test unikraft with 9pfs:

You can execute the `./run_unikraft.sh` script or follow the complete
procedure below:
    
1. For testing read latency, go to `apps/lib-readfile/`:
    - Run `./run_all.sh` (wait some times) 
    - Raw results will saved into `rhea-kvm_read.txt`;
    - Formatted results will be saved into `rhea-kvm_read.csv`;
2. For testing write latency, go to `apps/lib-writefile/`:
    - Run `./run_all.sh` (wait some times) 
    - Raw results will saved into `rhea-kvm_write.txt`;
    - Formatted results will be saved into `rhea-kvm_write.csv`;
    - Note that this experiment may take some time.

### Test VM Linux with 9pfs (require some manual steps):

You can execute the `./run_vm_linux.sh` script or follow the complete
procedure below:

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

### Generate the graph

Once all the results are generated, run the `./plot.sh` to generate
the 9pfs latency plot. This one will be generated in the current
folder with the following name: `compare_9pfs_lines.pdf`.