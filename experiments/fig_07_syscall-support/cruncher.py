#!/usr/bin/env python3
#pip3 install numpy
#pip3 install xlrd==1.2.0
#pip3 install pandas
#pip3 install seaborn (https://seaborn.pydata.org/generated/seaborn.heatmap.html#seaborn.heatmap)
#pip3 install matplotlib

import os
import sys
import json
import argparse
import xlrd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker


# Folder storing application JSON files from syscall analysis tool:
# one JSON file per-application
APPLICATION_JSON_FOLDER = 'aggregated_dockerfile'

# Excel file that contains the syscall implementation
# TODO: Use Google DOC API.
SHEET_FILENAME = 'Unikraft - Syscall Status.xlsx'

# Columns to consider in the excel file
NB_COLS = 3

INDEX_RAX = 0 #syscall number (col: 0)
INDEX_NAME = 1 #syscall name   (col: 1)
INDEX_STATUS = 2 #syscall status (col: 2)

# Keys in JSON files
STATIC_DATA = "static_data"
DYNAMIC_DATA = "dynamic_data"
SYSCALLS_DATA = "system_calls"


# Applications dictionary, indexed by application name
# Each item in the dictionary is another dictionary with list of system calls
# in the various states. Each inner dictionary uses the status as key: "OKAY",
# "ABSENT", "NOT_IMPL", "INCOMPLETE", "REG_MISS", "STUBBED", "BROKEN",
# "IN_PROGRESS", "PLANNED".
apps = {}

# System calls dictionary, indexed by syscall name
# Each item in the dictionary is another dictionary with the fields:
#   "id": system call id (number)
#   "name": system call name (again - also used as dictionary key)
#   "status": system call status
#   "apps": list of application names
#   "num_apps": number of applications using the syscall
syscalls = {}

# Undefined system calls dictionary, indexed by syscall name
# System calls that are undefined in the current system call list.
# This is the case with 32bit system calls - resulting from the investigation,
# but not as part of the system call status document.
# Each item in the dictionary is another dictionary with the fields:
#   "name": system call name (again - also used as dictionary key)
#   "apps": list of application names
#   "num_apps": number of of applications using the syscall
undefined_syscalls = {}


def process_application_json(app_name, json_data):
    """Fill apps, syscalls and undefined_syscalls dictionaries.

    Extract syscall related information from each application JSON file.
    Application name and application JSON data are passed as arguments.
    """

    # Extract all system calls in the JSON file in local_set.
    local_set = set()

    # First parse static data.
    if STATIC_DATA in json_data:
        static_data = json_data[STATIC_DATA][SYSCALLS_DATA]
        for symbol in static_data:
            local_set.add(symbol)

    # Then parse dynamic data.
    if DYNAMIC_DATA in json_data:
        dynamic_data = json_data[DYNAMIC_DATA][SYSCALLS_DATA]
        for symbol in dynamic_data:
            local_set.add(symbol)

    # Construct application dictionary (apps).
    apps[app_name] = {
        "OKAY": [],
        "ABSENT": [],
        "NOT_IMPL": [],
        "INCOMPLETE": [],
        "REG_MISS": [],
        "STUBBED": [],
        "BROKEN": [],
        "IN_PROGRESS": [],
        "PLANNED": []
        }
    for symbol in local_set:
        if symbol in syscalls.keys():
            status = syscalls[symbol]['status']
        else:
            status = "ABSENT"
        apps[app_name][status].append(symbol)

    # Update syscall dictionary with application list.
    # Construct undedefine_syscalls dictionary.
    for symbol in local_set:
        found = False
        for s in syscalls:
            if s == symbol:
                found = True
                break
        if found:
            syscalls[symbol]['apps'].append(app_name)
            syscalls[symbol]['num_apps'] += 1
        else:
            if not symbol in undefined_syscalls:
                undefined_syscalls[symbol] = {
                    'name': symbol,
                    'apps': [],
                    'num_apps': 0
                    }
            undefined_syscalls[symbol]['apps'].append(app_name)
            undefined_syscalls[symbol]['num_apps'] += 1


def walk_application_json_folder(path):
    """Walk folder with application JSON files.

    Read each per-application JSON file and process it.
    """

    for subdir, _, files in os.walk(path):
        for file in sorted(files):
            filepath = subdir + os.sep + file

            if filepath.endswith(".json"):
                with open(filepath) as json_file:
                    json_data = json.load(json_file)
                    process_application_json(file[:-5], json_data)


def process_syscall_spreadsheet(filename):
    """Interpret syscall status spreadsheet (.xls).

    Only extract the first three columns (cols: [0-2]).
    Columns are:
      * column 0 (INDEX_RAX): system call id (number)
      * column 1 (INDEX_NAME): system call name
      * column 2 (INDEX_STATUS): system call status

    Return value is data_sheet, a list of three lists, one for each column.
    """

    data_sheet = list()

    book = xlrd.open_workbook(filename)
    worksheet = book.sheet_by_index(0)

    # Init the data_sheet with 3 sublists
    for _ in range(NB_COLS):
        data_sheet.append(list())

    # Populate data_sheet with cell values (COLS: 0, 1, 2)
    for row in range(1, worksheet.nrows):
        try:
            data_sheet[INDEX_RAX].append(int(
                worksheet.cell_value(row, INDEX_RAX)))
        except ValueError:
            # This is not a number, change it to -1
            data_sheet[INDEX_RAX].append(-1)

        data_sheet[INDEX_NAME].append(
            worksheet.cell_value(row, INDEX_NAME))

        status_str = worksheet.cell_value(row, INDEX_STATUS)
        if len(status_str) == 0:
            status_str = 'NOT_IMPL'
        elif 'incomplete' in status_str:
            status_str = "INCOMPLETE"
        elif 'registration missing' in status_str:
            status_str = "REG_MISS"
        elif 'stubbed' in status_str:
            status_str = "STUBBED"
        elif 'planned' in status_str:
            status_str = "PLANNED"
        elif 'progress' in status_str:
            status_str = "IN_PROGRESS"
        elif 'broken' in status_str:
            status_str = "BROKEN"
        elif 'okay' in status_str:
            status_str = "OKAY"
        data_sheet[INDEX_STATUS].append(status_str)

    return data_sheet


def print_apps():
    """Print apps dictionary as comma-separated values (CSV).
    """
    print("{},{},{},{},{},{},{},{},{},{},{}".format(
        "app", "total", "okay", "not_impl", "reg_miss", "incomplete",
        "stubbed", "planned", "broken", "in_progress", "absent"))
    for a in apps:
        okay = len(apps[a]['OKAY'])
        not_impl = len(apps[a]["NOT_IMPL"])
        incomplete = len(apps[a]["INCOMPLETE"])
        reg_miss = len(apps[a]["REG_MISS"])
        stubbed = len(apps[a]["STUBBED"])
        planned = len(apps[a]["PLANNED"])
        in_progress = len(apps[a]["IN_PROGRESS"])
        broken = len(apps[a]["BROKEN"])
        absent = len(apps[a]["ABSENT"])
        total = okay + not_impl + incomplete + reg_miss + stubbed + planned + in_progress + broken
        print("{},{},{},{},{},{},{},{},{},{},{}".format(
            a, total, okay, not_impl, reg_miss, incomplete,
            stubbed, planned, broken, in_progress, absent))


def print_syscalls():
    """Print system calls from syscalls and undefined_syscalls dictionary
    as comma-separated values (CSV).
    """
    print("{},{},{}".format("syscall", "status", "num_apps"))
    for s in syscalls:
        print("{},{},{}".format(s, syscalls[s]['status'], len(syscalls[s]['apps'])))
    for s in undefined_syscalls:
        print("{},{},{}".format(s, 'ABSENT', len(undefined_syscalls[s]['apps'])))


def top_not_supported_syscalls(top):
    """Extract top not-supported syscalls.
    """
    sorted_syscalls = sorted(syscalls, key=lambda a: syscalls[a]['num_apps'], reverse=True)
    top_list = []
    for s in sorted_syscalls:
        if syscalls[s]['status'] == 'NOT_IMPL' \
                or syscalls[s]['status'] == 'PLANNED':
                    top_list.append(s)
                    if len(top_list) >= top:
                        break

    return top_list


def get_not_supported_except(app, except_list):
    not_impl = len(apps[app]["NOT_IMPL"])
    incomplete = len(apps[app]["INCOMPLETE"])
    reg_miss = len(apps[app]["REG_MISS"])
    stubbed = len(apps[app]["STUBBED"])
    planned = len(apps[app]["PLANNED"])
    in_progress = len(apps[app]["IN_PROGRESS"])
    broken = len(apps[app]["BROKEN"])
    absent = len(apps[app]["ABSENT"])

    not_supported_list = apps[app]["NOT_IMPL"] + apps[app]["PLANNED"]
    initial = len(not_supported_list)

    for s in except_list:
        if s in not_supported_list:
            initial -= 1

    return initial


app_syscalls_supported = {}


def collect_app_syscalls_supported():
    """Collect statistics on system call support per application.
    as comma-separated values (CSV).
    """
    top_5_not_supported = top_not_supported_syscalls(5)
    top_10_not_supported = top_not_supported_syscalls(10)
    #print(top_5_not_supported)
    #print(top_10_not_supported)
    for a in apps:
        okay = len(apps[a]['OKAY'])
        not_impl = len(apps[a]["NOT_IMPL"])
        incomplete = len(apps[a]["INCOMPLETE"])
        reg_miss = len(apps[a]["REG_MISS"])
        stubbed = len(apps[a]["STUBBED"])
        planned = len(apps[a]["PLANNED"])
        in_progress = len(apps[a]["IN_PROGRESS"])
        broken = len(apps[a]["BROKEN"])
        absent = len(apps[a]["ABSENT"])
        total = okay + not_impl + incomplete + reg_miss + stubbed + planned + in_progress + broken
        not_supported = not_impl + planned
        not_supported_except_top_5 = get_not_supported_except(a, top_5_not_supported)
        not_supported_except_top_10 = get_not_supported_except(a, top_10_not_supported)
        supported = okay + reg_miss + stubbed + incomplete + broken + in_progress
        app_syscalls_supported[a] = {
                "app": a,
                "total": total,
                "num_supported": supported,
                "perc_supported": supported * 100.0 / total,
                "num_not_supported": not_supported,
                "perc_not_supported": not_supported * 100.0 / total,
                "num_not_supported_except_top_5": not_supported_except_top_5,
                "perc_not_supported_except_top_5": not_supported_except_top_5 * 100.0 / total,
                "num_not_supported_except_top_10": not_supported_except_top_10,
                "perc_not_supported_except_top_10": not_supported_except_top_10 * 100.0 / total,
                }


def print_app_syscalls_supported():
    """Print syscall support as comma-separated values (CSV).
    """
    print("{},{},{},{},{}".format("app", "num_supported", "num_not_supported", "perc_supported", "perc_not_supported"))
    for a in app_syscalls_supported:
        print("{},{}/{},{}/{},{:4.2f},{:4.2f}".format(a["app"], a["num_supported"], a["total"], a["num_not_supported"], a["total"], a["perc_supported"], a["perc_not_supported"]))


def print_syscall_percentage_required():
    """Print the number of apps that require at least a given percentage of
    syscalls to be implemented.
    """
    print("{},{}".format("syscall_percentage","num_apps"))
    for i in range(0,101,5):
        num = 0
        for a in app_syscalls_supported:
            if a["perc_supported"] > i:
                num += 1
        print("{:d}%,{:d}".format(i, num))


def plot_syscall_support_cdf():
    """Plot CDF (Cumulative Distribution Function) that highlights
    syscall support.
    """
    syscall_percentage_list = []
    app_percentage_list = []
    for i in range(0,101,5):
        num = 0
        for a in app_syscalls_supported:
            if a["perc_supported"] > i:
                num += 1
        syscall_percentage_list.append("{:d}%".format(i))
        app_percentage_list.append(num * 100 / len(app_percentage))

    print(app_percentage_list)
    print(syscall_percentage_list)
    #fig, ax = plt.subplots()
    plt.bar(syscall_percentage_list, app_percentage_list)
    #ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%3.0f%%"))
    #ax.set(xlim=(0, 1), ylim=(0, 1))
    plt.xlabel('Supported syscalls [%]')
    plt.ylabel('Applications with at least that syscall support [%]')
    plt.grid(True)

    plt.tight_layout()
    plt.show()


def plot_syscall_support():
    """Print plot to highlight syscall support.
    """
    syscall_percentage_list = []
    app_percentage_list = []
    for i in range(0,101,5):
        num = 0
        for a in app_syscalls_supported:
            if a["perc_supported"] >= i and a["perc_supported"] < i+5:
                num += 1
        syscall_percentage_list.append("{:d}%".format(i))
        app_percentage_list.append(num * 100 / len(app_syscalls_supported))

    print(app_percentage_list)
    print(syscall_percentage_list)
    fig, ax = plt.subplots()
    plt.bar(syscall_percentage_list, app_percentage_list)
    ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%3.0f%%"))
    #ax.set(xlim=(0, 1), ylim=(0, 1))
    plt.xlabel('Supported syscalls [%]')
    plt.ylabel('Applications with that much syscall support [%]')
    plt.grid(True)

    plt.tight_layout()
    plt.show()


def plot_syscall_support_per_app():
    """Print plot to highlight syscall support per application.
    """
    supported_list = []
    not_supported_list = []
    not_supported_5_list = []
    not_supported_10_list = []
    only_5_list = []
    only_10_list = []
    only_5_to_10_list = []
    app_list = []
    for a in app_syscalls_supported:
        app_list.append(a)
        supported = app_syscalls_supported[a]["perc_supported"]
        not_supported = app_syscalls_supported[a]["perc_not_supported"]
        not_supported_5 = app_syscalls_supported[a]["perc_not_supported_except_top_5"]
        not_supported_10 = app_syscalls_supported[a]["perc_not_supported_except_top_10"]
        only_5 = not_supported - not_supported_5
        only_10 = not_supported - not_supported_10
        only_5_to_10 = not_supported_5 - not_supported_10
        supported_list.append(supported)
        not_supported_list.append(not_supported)
        not_supported_5_list.append(not_supported_5)
        not_supported_10_list.append(not_supported_10)
        only_5_list.append(only_5)
        only_10_list.append(only_10)
        only_5_to_10_list.append(only_5_to_10)

    fig, ax = plt.subplots()
    #p1 = plt.bar(app_list, supported_list, color='white', edgecolor='black') #, hatch='x')
    #p2 = plt.bar(app_list, not_supported_list, bottom=supported_list, color='black', edgecolor='black') #, hatch='/')
    #p3 = plt.bar(app_list, only_10_list, bottom=supported_list, color='gray', edgecolor='black') #, hatch='+')
    #p4 = plt.bar(app_list, only_5_list, bottom=supported_list, color='lightgray', edgecolor='black') #, hatch='*')
    p1 = plt.bar(app_list, supported_list)
    p2 = plt.bar(app_list, not_supported_list, bottom=supported_list)
    p3 = plt.bar(app_list, only_10_list, bottom=supported_list)
    p4 = plt.bar(app_list, only_5_list, bottom=supported_list)

    ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%3.0f%%"))
    ax.yaxis.grid(linestyle='dotted')
    plt.xticks(np.arange(len(app_list)), app_list, rotation=45)
    plt.yticks(np.arange(0, 101, 10))
    #plt.title('System call support per-application')
    #plt.xlabel('Applications')
    plt.ylabel('System call support', fontsize=16)
    plt.legend((p1[0], p4[0], p3[0], p2[0]), ('Supported syscalls', 'If top 5 syscalls implemented', 'If top 10 syscalls implemented', 'If remaining syscalls implemented'))

    plt.savefig("syscall-support.pdf")


def main():
    """Process system call status spreadsheet (SHEET_FILENAME) and
    application JSON folder (APPLICATION_JSON_FOLDER).
    Print application and / or system call statistics depending on arguments.
    """

    parser = argparse.ArgumentParser()
    parser.add_argument('-a', '--apps', action='store_true',
                        help='Print system call support in applications')
    parser.add_argument('-s', '--syscalls', action='store_true',
                        help='Print system call usage / popularity in apps')
    parser.add_argument('-p', '--plot', action='store_true',
                        help='Plot syscall support')
    parser.add_argument('-m', '--missing', action='store_true',
                        help='Percentage of syscalls missing per app')
    args = parser.parse_args()

    data_sheet = process_syscall_spreadsheet(SHEET_FILENAME)
    ids = data_sheet[0]
    names = data_sheet[1]
    stats = data_sheet[2]

    for i in range(len(ids)):
        syscalls[names[i]] = {
            "id": ids[i],
            "name": names[i],
            "status": stats[i],
            "apps": [],
            "num_apps": 0
            }
    # Read folder with application JSON files and aggregate the data.
    walk_application_json_folder(APPLICATION_JSON_FOLDER)

    # Collect statistics about supported system calls for each app.
    collect_app_syscalls_supported()

    if args.apps:
        print_apps()
    if args.syscalls:
        print_syscalls()
    if args.missing:
        print_app_syscalls_supported()
        print_syscall_percentage_required()
    if args.plot:
        plot_syscall_support_per_app()


if __name__ == "__main__":
    sys.exit(main())