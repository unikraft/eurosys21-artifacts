# abi compatibility

## Goal

This repository contains the experiments for Table 2 (ABI
compatibility). Since these experiments have a heavy size, we decided
to host the zip file which contains the setup on another server.

**APPROXIMATIVE EXECUTION TIME=30/45minutes**

## How to Use (automatic test)

You can automatically run all the experiment by running the `make all`
command. This command will:

1. Prepare the environment by executing the `./setup.sh` script. Note
that this script takes a long time...
2. Run the ABI compatibility experiment by running `./run_all.sh`. 
3. Generate the table by running the `./plot.sh` script. It will be
saved into the current repository as `merged.csv`.

## How to Use (manual test)

You can also test manually by executing the following procedure.

In order to setup the environment for these experiments, please
execute first the `./setup.sh` script. This script will setup all the
environment to test the ABI compatibility. Once the setup is done, you
have three possibilities. Note that the setup takes time since each
application must be compiled with Unikraft.

### Test with musl

To test with `musl`, enter the following commands from the experiment
folder:
```
cp script_abi/script_run.sh abi/apps_musl/
cd abi/apps_musl/
./script_run.sh musl-abi
```
Sizes of all entries will be computed automatically by the script and
will be printed on stdout. In addition, there are also saved into a
CSV file called `musl-abi.csv` and located at `apps_musl/`.

### Test with newlib

To test with `newlib`, enter the following commands from the experiment
folder:
```
cp script_abi/script_run.sh abi/apps_newlib/
cd abi/apps_newlib/
./script_run.sh newlib-abi
```
Sizes of all entries will be computed automatically by the script and
will be printed on stdout. In addition, there are also saved into a
CSV file called `newlib-abi.csv` and located at `apps_newlib/`.

### Getting LoC

To get the #LoC which is displayed on Table 2, enter the following
commands from the experiment folder:
```
cp script_abi/script_cloc.sh abi/apps_loc/
cd abi/apps_loc/
./script_cloc.sh
```
Output will be displayed on stdout and saved into a CSV file
`cloc.csv` and located at `apps_loc/`.

### Generate the CSV file

Once all the results are generated, run the `./plot.sh` script to
generate the table 2 as a CSV file. This one will be generated in the
current folder with the following name: `merged.csv`.