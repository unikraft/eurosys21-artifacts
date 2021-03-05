import csv


with open('tx_only_1024B_ds', newline='') as f:
    with open('tx_only_1024B_csv', newline='') as g:
        out_data = []
        writer = csv.writer(g)
        reader = csv.reader(f)
        count = 1
        for row in reader:
            for i in row:
                if i != "": 
                    print(count, i)
                    writer.writerow([str(count), str(i)])
                    count = count + 1
