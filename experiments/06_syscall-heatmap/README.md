# syscalls-heatmap

This repository contains a python script to generate a syscalls
heatmap for Unikraft. An excel file which contains the current
syscalls status is provided in this repository.

# Installation

```
pip3 install numpy
pip3 install xlrd==1.2.0
pip3 install pandas
pip3 install seaborn 
pip3 install matplotlib
```
# Usage

Just install the required libraries (see above) and then run it with: 
`python3 heatmap.py` to generate the heatmap.

# Gathering syscalls for a new application

It is quite difficult to automate the task of gathering all the 
syscalls of a specific application. It is why we already did the 
analysis of 30 applications. The results of the analysis are locacted
within the `to_aggregate` folder. In that case, a static and dynamic
analysis have been performed on all these apps by our toolchain (see
below).

If you want to gather yourself the syscalls of an app, you need to
download and use our toolchain. Please follow the following steps:
1. clone the tools repo with the following command: `git clone git@github.com:gaulthiergain/tools.git`;
2. install GO and setup your GO PATH/ROOT variables;
3. run `make deps` within this folder and then run `make`;
4. when you have the `tools` binary file, you can use it as follow:
`./tools --dep -p <program_name> [-o|--options <option>][-t|--testFile <test_file>]`;
5. Note that you need to install yourself the application before using the tools;

Example with the apache web server, a specic config file and a test file.

`sudo ./tools --dep -p ~/dev/bin_programs/apache/httpd-2.4.46/httpd -o " -f /usr/local/apache2/conf/httpd.conf -X -k start" -t ~/go/src/tools/testfiles/test_http.json`

Please refer to the wiki for further information: https://github.com/gaulthiergain/tools/wiki

# Further Information

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
