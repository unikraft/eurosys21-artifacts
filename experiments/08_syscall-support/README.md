# syscalls-cruncher

This repository contains a python script to generate a syscalls
heatmap for Unikraft. An excel file which contains the current
syscalls status is provided in this repository.

## Installation

```
pip3 install numpy
pip3 install xlrd==1.2.0
pip3 install pandas
pip3 install seaborn 
pip3 install matplotlib
```
## Usage

Just install the required libraries (see above) and then run it with: 
`python3 cruncher.py --plot` to generate the plot.

## Gathering syscalls for a new application

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

## Further Information

```
usage: cruncher.py [-h] [-a] [-s] [-p] [-m]

optional arguments:
  -h, --help      show this help message and exit
  -a, --apps      Print system call support in applications
  -s, --syscalls  Print system call usage / popularity in apps
  -p, --plot      Plot syscall support
  -m, --missing   Percentage of syscalls missing per app
```

Note that if you use the `--aggregated-file` argument, you need to 
adapt the `--nb-apps` argument. Its default value is 30.

To get numerical system call statistics in applications use `cruncher.py`. Use `-s` or `-a` arguments to print system call popularity or system call support in applications:

```
$ python cruncher.py -s
syscall,status,num_apps
read,OKAY,30
[...]

$ python cruncher.py -a
app,total,okay,not_impl,reg_miss,incomplete,stubbed,planned,broken,in_progress,absent
apache,41,23,8,1,5,1,0,1,2,1
[...]
```
