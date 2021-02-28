# abi compatibility

This repository contains the experiments for table 2. Since these experiments
use a lof of object files, we decided to create a zip file which contains
all the setup used for these experiments. In order to perform again these
experiments, please follow the steps below:
1. Download the zip archive: `wget https://people.montefiore.uliege.be/gain/unikraft/table2-experiments.zip`
2. Unzip it: `unzip table2-experiments.zip`
3. You can now test either with newlib or musl.
    3.1. To test with `newlib`, just go to `abi/apps_newlib` and run `./script.sh`. Sizes of all entries will be computed automatically by the script and will be printed on stdout.
    3.2. To test with `musl`, just go to `abi/apps_musl` and run `./script.sh`. Sizes of all entries will be computed automatically by the script and will be printed on stdout.

Note that these experiments require some time since each application must be
compiled with Unikraft.
