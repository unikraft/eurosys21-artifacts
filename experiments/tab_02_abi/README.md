### abi compatibility

This repository contains the experiments for table 2 (abi
compatibility). Since these experiments have a heavy size, we decided
to host the zip file which contains the setup on another server.

In order to setup the environment for these experiments, please
execute first the `./setup.sh` script. This script will setup all the
environement to test the abi compatibility. Once the setup is done,
you have two choices.

Note that these experiments require some time since each application
must be compiled with Unikraft.

## Test with musl

To test with `musl`, just go to `apps_musl` and run `./script.sh`.
Sizes of all entries will be computed automatically by the script and
will be printed on stdout.

## Test with newlib

To test with `newlib`, just go to `apps_newlib` and run `./script.sh`.
Sizes of all entries will be computed automatically by the script and
will be printed on stdout.
