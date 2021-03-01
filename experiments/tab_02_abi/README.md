### abi compatibility

This repository contains the experiments for Table 2 (abi
compatibility). Since these experiments have a heavy size, we decided
to host the zip file which contains the setup on another server.

In order to setup the environment for these experiments, please
execute first the `./setup.sh` script. This script will setup all the
environement to test the abi compatibility. Once the setup is done,
you have three choices.

## Test with musl (this experiment takes time)

To test with `musl`, just go to `apps_musl/` and run `./script.sh`.
Sizes of all entries will be computed automatically by the script and
will be printed on stdout. In addition, there are also saved into 
a CSV file called `musl-abi.csv` and located at `apps_musl/`.

Note that these experiments require some time since each application
must be compiled with Unikraft.

## Test with newlib (this experiment takes time)

To test with `newlib`, just go to `apps_newlib/` and run `./script.sh`.
Sizes of all entries will be computed automatically by the script and
will be printed on stdout. In addition, there are also saved into 
a CSV file called `musl-newlib.csv` and located at `apps_newlib/`.

Note that these experiments require some time since each application
must be compiled with Unikraft.

## Getting LoC

To get the #LoC which is displayed on Table 2, just go to `apps_loc/`
and run `./script.sh`. Output will be displayed on stdout and 
saved into a CSV file `cloc.csv` and located at `apps_loc/`.