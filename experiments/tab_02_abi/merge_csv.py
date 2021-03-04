import os
from os import listdir
from os.path import isfile, join

DATA = "data"
CLOC = "cloc.csv"
MERGED = "merged.csv"

def write_merged_file(filename, header, map_name, total_apps):
    string_builder = [""] * total_apps

    for ih, h in enumerate(header):
        for i,k in enumerate(map_name[h]):
            if ih == len(header)-1 :
                string_builder[i] += k + "\n"
            else:
                string_builder[i] += k + ","
    f = open(filename, 'w')
    f.write(','.join(header) + '\n')
    f.write(''.join(string_builder))
    f.close()

def read_content_loc(filename, map_name):
    total_apps = 0
    for line in open(filename, 'r'):
        l = line.strip()
        map_name['name'].append(l.split(",")[0])
        map_name['cloc'].append(l.split(",")[1])
        total_apps += 1
    return total_apps

def read_content_std(filename, map_name, header_name):
    for i,line in enumerate(open(filename, 'r')):
        l = line.strip()
        map_name[header_name + '-std'].append(l.split(",")[1])

def read_content_compat(filename, map_name, header_name):
    for i,line in enumerate(open(filename, 'r')):
        l = line.strip()
        map_name[header_name + '-size'].append(l.split(",")[1])
        map_name[header_name + '-compat'].append(l.split(",")[2])

def read_csv_files(folder, map_name):
    for f in sorted(listdir(folder)):
        filename = join(folder, f)
        if isfile(filename) and filename.endswith('.csv') and MERGED not in filename:
            if "musl-std" in filename:
                read_content_std(filename, map_name, 'musl')
            elif "musl-compat" in filename:
                read_content_compat(filename, map_name, 'musl')
            elif "newlib-std" in filename:
                read_content_std(filename, map_name, 'newlib')
            elif "newlib-compat" in filename:
                read_content_compat(filename, map_name, 'newlib')
            elif "cloc" in filename:
                total_apps = read_content_loc(filename, map_name)

    return total_apps

def main():

    header = ['name','musl-size','musl-std','musl-compat','newlib-size','newlib-std','newlib-compat','cloc']
    map_name = dict()
    for h in header:
        map_name[h] = list()

    total_apps = read_csv_files(DATA, map_name)
    write_merged_file(MERGED, header, map_name, total_apps)

if __name__ == "__main__":
    main()
