import os
from os import listdir
from os.path import isfile, join

DATA = "data"
CLOC = "cloc.csv"
MERGED = "merged.csv"

def write_merged_file(filename, rows):
    f = open(filename, 'w')
    header = "name,cloc,musl,newlib\n"
    f.write(header)
    for line in rows:
         f.write(line + "\n")
    f.close()

def read_content(filename, rows):
    for i,line in enumerate(open(filename, 'r')):
        l = line.strip()
        if CLOC in filename:
            rows.append(l)
        else:
            rows[i]+=","+l.split(",")[1]

def read_csv_files(folder, rows):
    for f in sorted(listdir(folder)):
        filename = join(folder, f)
        if isfile(filename) and filename.endswith('.csv') and MERGED not in filename:
            read_content(filename, rows)

def main():
    rows = list()
    read_csv_files(DATA, rows)
    write_merged_file(MERGED, rows)

if __name__ == "__main__":
    main()

