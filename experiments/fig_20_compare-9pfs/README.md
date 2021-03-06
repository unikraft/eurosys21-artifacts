# 9pfs latency

## Goal

This folder is used for a performance experiment of 9pfs between
[Unikraft][] and a Linux VM (write and read latency). Since these
experiments require a large VM, we decided to host the zip file which
contains the Linux VM on another [server]. If the server is down
you can download it from the [backup] server.

[Unikraft]: https://github.com/unikraft/unikraft
[server]: http://releases.unikraft.org/.eurosys21/vm.zip
[backup]: https://people.montefiore.uliege.be/gain/unikraft/vm.zip

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
    - Raw results will saved into `unikraft-kvm_read.txt`;
    - Formatted results will be saved into `unikraft-kvm_read.csv`;
2. For testing write latency, go to `apps/lib-writefile/`:
    - Run `./run_all.sh` (wait some times) 
    - Raw results will saved into `unikraft-kvm_write.txt`;
    - Formatted results will be saved into `unikraft-kvm_write.csv`;
    - Note that this experiment takes some time.

### Test VM Linux with 9pfs (require some manual steps):

You can execute the `./run_vm_linux.sh` script or follow the complete
procedure below:

1. Go to the `vm` folder;
2. Run the launcher script (`./launcher.sh`);
3. The VM and the experiment are automatically started:
    - Formatted results will be saved into `test/linux-kvm_read.csv`
    and `test/linux-kvm_write.csv`. They will be accessible from the
    host in the `test/`;
    - The guest VM will be automatically stopped at the end of the
    script;
    - Note that this experiment takes a lot of time.

### Generate the graph

Once all the results are generated, run the `./plot.sh`, then the
`plot.py` script to generate the 9pfs latency plot. This one will
be generated in the current folder with the following name: 
`fig_20_compare-9pfs.svg`.

### Further information about the VM

The VM has been created from a linux kernel 4.15.0-96-generic. We
didn't patch or modify the kernel code. We just added a 9pfs mount
point (`test`) which allows to share data between the host and the
guest. In addition, all sources are also provided in the `vm` folder.

If the autologin failed, the login/password is `root` and the
experiment script is `./script.sh`.
