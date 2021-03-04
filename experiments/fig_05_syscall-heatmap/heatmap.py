#!/usr/bin/env python3
#pip3 install numpy
#pip3 install xlrd==1.2.0
#pip3 install pandas
#pip3 install seaborn (https://seaborn.pydata.org/generated/seaborn.heatmap.html#seaborn.heatmap)
#pip3 install matplotlib

import os
import sys
import json
import xlrd
import argparse
import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt

class colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

# Sycalls occurences used by apps (default value).
SYSCALLS_FILENAME = 'syscalls_sample.json'

# Number of apps that were analysed (default value).
NB_APPS = 30


# Excel file that contains the syscall implementation.
#TODO USE Google DOC API.
SHEET_FILENAME = 'Unikraft - Syscall Status.xlsx'

# Columns to consider in the excel file.
NB_COLS = 3

INDEX_RAX       = 0 #syscall number (col: 0)
INDEX_NAME      = 1 #syscall name   (col: 1)
INDEX_STATUS    = 2 #syscall status (col: 2)

# Width of the grid (15 cells).
NB_DIV = 15

# Font size for text in cell.
FONT_SIZE = 8

# For JSON parsing.
STATIC_DATA = "static_data"
DYNAMIC_DATA = "dynamic_data"
SYSCALLS_DATA = "system_calls"

# readAggregatedFile reads an aggregated json file which contains the 
# mapping <syscall, usage>. It is used with the argument
# '--aggregated-file'.
def readAggregatedFile(data, path):

    with open(path) as json_file:
        json_data = json.load(json_file)
        for key, value in json_data.items():
            if key in data:
                data[key] = value
            else:
                # May require local change to the json file to adapt the name.
                print("[WARNING]" + symbol + " is not present in the excel file.")

# processToAggregate merges the dynamic and static json keys into
# a single json object.
def processToAggregate(file, data, json_data):

    # Used to keep track of syscalls of a single json file.
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

    for symbol in local_set:
        if symbol in data:
            data[symbol] += 1
        else:
            # May require local change to the json file to adapt the name.
            print(f'[{colors.WARNING}WARNING{colors.ENDC}] Symbol {colors.UNDERLINE}{symbol}{colors.ENDC} (in file {colors.BOLD}{file}{colors.ENDC}) is not present in the EXCEL file')

# readAggregateFile reads an aggregated json file which contains the 
# mapping <syscall, usage>. It is used with the argument 
# '--aggregated-file'.
def aggregateFolder(data, path):
    
    nbFiles = 0
    for subdir, dirs, files in os.walk(path):
        for file in sorted(files):
            filepath = subdir + os.sep + file

            if filepath.endswith(".json"):
                with open(filepath) as json_file:
                    json_data = json.load(json_file)
                    processToAggregate(file, data, json_data)
                    nbFiles = nbFiles + 1

    return nbFiles

# str2bool is used for boolean arguments parsing.
def str2bool(v):
    
    if isinstance(v, bool):
       return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

# openExcelSheet opens an excel sheet and reads its content.
# Only the first three columns are parsed (cols: [0-2]).
def openExcelSheet(filename):

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
        except:
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
        data_sheet[INDEX_STATUS].append(status_str)

    return data_sheet

# chunks splits a list into a list of lists (matrix) of a specific size.
def chunks(list_to_split, chunk_size):
    
    result_list = []
    while list_to_split:
        result_list.append(list_to_split[:chunk_size])
        list_to_split = list_to_split[chunk_size:]
    return result_list

list_values = list()
# plot displays the heapmap.
def plot(data, data_sheet, labels, display_syscall_name, save_heatmap):
    
    ax = sns.heatmap(np.array(data), 
        annot=labels,
        square=False,
        cmap = sns.cm.rocket_r, 
        vmax=100,
        fmt='s', 
        linewidths=.5, 
        yticklabels=False, 
        xticklabels=False,
        annot_kws={'size':FONT_SIZE})

    for i,t in enumerate(ax.texts): 
        if data_sheet[INDEX_STATUS][i].upper() == "OKAY":
            #print(data_sheet[INDEX_STATUS][i].upper())
            t.set_text(t.get_text())
            #print(str(data_sheet[INDEX_RAX][i]) + ":" + data_sheet[INDEX_NAME][i] + " : " + data_sheet[INDEX_STATUS][i])
        else:
            t.set_text("")
            
    if display_syscall_name:
        if len(data_sheet[INDEX_STATUS]) > 0:
            for i,t in enumerate(ax.texts): t.set_text(t.get_text() + "\n" + str(data_sheet[INDEX_NAME][i]) + "\n[" +data_sheet[INDEX_STATUS][i].upper() + "]")
        else:
            for i,t in enumerate(ax.texts): t.set_text("")
    if save_heatmap:
        plt.savefig("syscall-heatmap.svg")
        print("Figure saved into the current repo with the name syscall-heatmap.pdf")
    else:
        plt.show()

def main():

    parser = argparse.ArgumentParser()

    parser.add_argument('--aggregated-file', help='Path to the aggregated json file which contains data (default: ' + SYSCALLS_FILENAME + ')', default=SYSCALLS_FILENAME)
    parser.add_argument('--nb-apps', help='Numper of apps that were analysed by the toolchain (default: ' + str(NB_APPS) + ')', type=int, default=NB_APPS)
    parser.add_argument('--folder-to-aggregate', help= 'Path to the folder which contains several json files to aggregate them in the right way')
    parser.add_argument('--display-syscall-name', type=str2bool, 
                        nargs='?', const=True, default=False,
                        help='Display syscall\'s name on the heatmap (require a big display).')
    parser.add_argument('--save-heatmap', type=str2bool, 
                        nargs='?', const=True, default=False,
                        help='Save heatmap to PDF in the current folder.')
    
    args = parser.parse_args()
    data_sheet = openExcelSheet(SHEET_FILENAME)
    
    data = { i : 0 for i in data_sheet[INDEX_NAME]}
    if args.folder_to_aggregate is not None:
        # Read a folder and aggregate the data.
        args.nb_apps = aggregateFolder(data, args.folder_to_aggregate)
        with open('aggregated.json', 'w', encoding='utf-8') as f:
            json.dump(data, f, ensure_ascii=False, indent=4)
    else:
        # Read an aggregate file that contains already the mapping <syscall, usage>.
        readAggregatedFile(data, args.aggregated_file)

    for v in data.values():
        # Compute a percentage.
        list_values.append((v/args.nb_apps)*100)

    # Split into chunks the values.
    data = chunks(list_values, NB_DIV)

    # Use the syscall number a string for the annotation.
    labels = chunks(list(map(str,data_sheet[INDEX_RAX])), NB_DIV)
    
    plot(data, data_sheet, labels, args.display_syscall_name, args.save_heatmap)

if __name__ == "__main__":
    main()
