from statistics import mean


def get_mean(file_path):
    with open(file_path,'r') as fin:
        data=[float(x) for x in fin.read().split('\n') if x != '']
    average = mean(data)
    return average

import sys


m1 = get_mean("results/xen_9p.results")
m2 = get_mean("results/xen.results")

print("Results for XEN:")
print(str((m1 - m2) / 10 ** 6) + str(" ms"))


m1 = get_mean("results/kvm_9p.results")
m2 = get_mean("results/kvm.results")
print("Results for KVM:")
print(str((m1 - m2) / 10 ** 6) + str(" ms"))
