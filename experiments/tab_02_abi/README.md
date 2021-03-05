# abi compatibility

## Goal

This repository contains the experiments for Table 2 (ABI
compatibility). Since these experiments have a heavy size, we decided
to host the zip archive which contains the setup on another [server].
If the server is down you can download it from the [backup] server.

[Unikraft]: https://github.com/unikraft/unikraft
[server]: http://releases.unikraft.org/.eurosys21/abi.zip
[backup]: https://people.montefiore.uliege.be/gain/unikraft/abi.zip


**APPROXIMATIVE EXECUTION TIME=2h**

**NOTE THAT THIS EXPERIMENT REQUIRES SOME DISK SPACE (+/-1.5GB in the /tmp folder)**

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

After this setup go to `/tmp/abi` for the remaining procedures.

### Test with musl

To test with `musl`, enter the following commands from the experiment
folder:
```
cp script_abi/script_run.sh abi/apps_musl_compat/
cd abi/apps_musl_compat/
./script_run.sh musl-compat

cp script_abi/script_check.sh abi/apps_musl_std/
cd abi/apps_musl_std/
./script_check.sh musl-std
```

The first part will compute the size of all unikernels when the
compatibility layer is enabled.
The second part will display a checkmark (V) or a fail (V) when the
compatibility is disabled. A checkmark means that the application has
been successfully built. Otherwise, it will display a fail.

Info of all entries will be computed automatically by the script and
will be printed on stdout. In addition, there are also saved into
CSV files respectively called `musl-compat.csv` (in the folder
`apps_musl_compat/`) and `musl-std.csv` (in the folder
`apps_musl_std/`).


### Test with newlib

To test with `newlib`, enter the following commands from the experiment
folder:
```
cp script_abi/script_run.sh abi/apps_newlib_compat/
cd abi/apps_newlib_compat/
./script_run.sh newlib-compat

cp script_abi/script_check.sh abi/apps_newlib_std/
cd abi/apps_newlib_std/
./script_check.sh newlib-std
```

The first part will compute the size of all unikernels when the
compatibility layer is enabled.
The second part will display a checkmark (V) or a fail (V) when the
compatibility is disabled. A checkmark means that the application has
been successfully built. Otherwise, it will display a fail.

Info of all entries will be computed automatically by the script and
will be printed on stdout. In addition, there are also saved into
CSV files respectively called `newlib-compat.csv` (in the folder
`apps_newlib_compat/`) and `newlib-std.csv` (in the folder
`apps_newlib_std/`).

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


### Further information about the archive

The archive contains different folders:
- `apps_loc`: this folder contains the Unikraft application sources as
well as the experiments to count the number of lines of glue code 
which is necessary to port an application (from its sources) to
Unikraft. We use the *cloc* tool to count the number of lines.
- `apps_musl_compat`: this folder contains applications which use
*musl* as libc as well as the compat. layer which is called
`lib-compat`.
- `apps_musl_std`: this folder contains applications which use
*musl* as libc. The compat. layer is not used in that case.
- `apps_newlib_compat`: this folder contains application which uses
*newlib* as libc as well as the compat. layer which is called 
`lib-compat`.
- `apps_newlib_std`: this folder contains applications which use
*newlib* as libc. The compat. layer is not used in that case.
- `libs`: this folder contains Unikraft microlibs and more especially
all the microlibs which contains the objects built from their native
build system. These ones are prefixed with the `lib-objects-` keyword.
Within each `lib-object` folder, there is a `objs` folder which
contains the object files of the application (built with their native 
build system using gcc and glic). In addition, source files are also
provided.
- `unikraft`: this folder contains the main unikraft code.
