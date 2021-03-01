### 9pfs latency

This repository contains the experiments for the 9pfs latency. 
Since these experiments require a large VM, we decided to host the zip
file which contains the VM on another server.

In order to setup the environment for these experiments, please 
execute first the `./setup.sh` script. This script will setup all the
environement to test the 9pfs latency. Once the setup is done, you
have two choices:

## Test unikraft with 9pfs:
    1. Go to the `apps` folder;
    2. For testing write latency, go to `lib-writefile/` and execute:
          - ./runall.sh (wait some times, result is saved into `9pfs.txt`);
    3. For testing read latency, go to `lib-readfile/` and execute:
          - ./runall.sh (wait some times, result is saved into `9pfs.txt`);

## Test VM Linux with 9pfs:
    1. Go to the `vm` folder;
    2. Run the launcher script (`launcher.sh`);
    3. Enter login/pwd: `root/root`;
    4. In the VM prompt, run first the `mount9pfs.sh` script;
    5. Run the `readfile test/randomfile <BLOCK_SIZE>`, where `BLOCK_SIZE` can be 4,8,16,32,64;
    6. Run the `writefile test/<FILE> <BLOCK_SIZE>`, where `BLOCK_SIZE` can be 4,8,16,32,64 and `FILE` is a new file that will be generated (change name for each block size).
