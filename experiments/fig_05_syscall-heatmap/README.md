# Unikraft Syscall Heatmap

<img align="right" src="../../plots/fig_05_syscall-heatmap.svg" width="300" />

## Goal

This repository contains the experiment to generate a syscalls heatmap
for Unikraft. An excel file which contains the current syscalls status
is provided in this repository.

In order to gather syscalls we perform a syscalls analysis of 30
applications by running a dependency analyser tool that we have
implemented ourself (see *Others* section for futher information).

To have the right environment for each application, we have created a
docker file per application. These ones are available in the 
`dockerfiles` folder. Note that docker must be installed on your 
machine to perform this experiment.

**APPROXIMATIVE EXECUTION TIME=30/45minutes**

**This experiment requires at least docker client 20 and**
**docker engine 19 otherwise results may be less accurate**

## How to Use (automatic test)

You can automatically run all the experiment by running the `make all`
command. This command will:

1. Prepare the environment by executing the `./setup.sh` script. Note
that this script takes a long time...
2. Run the syscalls experiment by running `./run_all.sh`. 
3. Generate the heatmap by running the `./plot.sh` script and then the
figure is saved into the current repository as `fig_05_syscall-heatmap.svg`.

## How to Use (manual test)

You can also test manually by executing the following procedure.

In order to setup the environment for these experiments, please
execute first the `./setup.sh` script. This script will setup all the
environment to generate the heatmap. Once the setup is done, you
can run the dependency analyser by running the `./run_all.sh`. Note
that the setup takes time since each application must be built as a
docker container.

Once all the results are generated, run the `./plot.sh` script to
generate the heatmap. This one will be generated in the current folder
with the following name: `fig_05_syscall-heatmap.svg`.

## Others

Additional information that may be interesting if you want to test
a new application.

### Gathering syscalls for a new application

If you want to gather yourself the syscalls of an app, you need to
download and use our toolchain. Please follow the following steps:
1. Clone the tools repo with the following command: 
`git clone git@github.com:gaulthiergain/tools.git`;
2. Install GO and setup your GO PATH/ROOT variables;
3. Run `make deps` within this folder and then run `make`;
4. When you have the `tools` binary file, you can use it as follow:
`./tools --dep -p <program_name> [-o|--options <option>][-t|--testFile <test_file>]`;
5. Note that you need to install yourself the application before using 
the tools. The most easiest solution is to write a docker file.

Example with the nginx web server and a test file:

`./tools --dep -p nginx -t ~/testfiles/test_http.json`

Please refer to the wiki for further information: 
https://github.com/gaulthiergain/tools/wiki

### Further Information about the heatmap script

The heatmap script allows to generate has also various features:

```
python3 heatmap.py --help
usage: heatmap.py [-h] [--aggregated-file AGGREGATED_FILE] [--nb-apps NB_APPS]
                  [--folder-to-aggregate FOLDER_TO_AGGREGATE]
                  [--display-syscall-name [DISPLAY_SYSCALL_NAME]]
                  [--save-heatmap [SAVE_HEATMAP]]
```

There are two different modes:
1) **AGGREGATED_FILE**: Use the `--aggregated-file` argument following
by the path of a json file. This one contains an aggregated list of
syscalls that were gathered and aggregated by testing 30 applications.
The following file *"syscalls\_sample.json"* is provided as sample.
2) **FOLDER_TO_AGGREGATE**: Use the `--folder-to-aggregate` argument
following by the path of a folder that contains the json files
gathered by the [toolchain](https://github.com/gaulthiergain/tools).
These ones must use a particular structure that is defined by the
[toolchain](https://github.com/gaulthiergain/tools). An example folder
*"to\_aggregate"* is provided as sample.

Note that if you use the `--aggregated-file` argument, you need to 
adapt the `--nb-apps` argument. Its default value is 30.
