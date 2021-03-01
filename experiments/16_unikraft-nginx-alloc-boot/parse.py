#!/usr/bin/python3

from os import listdir, makedirs
from os.path import isfile, join
from copy import deepcopy
import collections
import sys, re

size = "1024M"

functions = {
    "pci-bus":          "libkvmpci_uk_bus_register",
    "virtio-bus":       "libkvmvirtio_uk_bus_register",
    "virtio-pci":       "libkvmvirtio_pci_register_driver",
    "virtio-net":       "libkvmvirtionet_virtio_register_driver",
    "swrand":           "_uk_swrand_init",
    "vfscore-fdtable":  "fdtable_init",
    "vfscore-init":     "vfscore_init",
    "uklibparam-vfs":   "vfsprocess_arg",
    "uklibparam-net":   "netdevprocess_arg",
    "posix-user":       "init_posix_user",
    "ukbus":            "uk_bus_init_all",
    "pthread-embedded": "pthread_initcall",
    "lwip":             "liblwip_init",
    "vfscore-rootfs":   "vfscore_rootfs"
}

rawfiles = [f for f in listdir("rawdata/" + size) if isfile(join("rawdata/" + size, f))]
rawfiles.sort()

dic = collections.OrderedDict()
blank = collections.OrderedDict()
blank = {"plat": 0, "virtio": 0, "vfscore": 0, "alloc": 0, "ukbus": 0, "pthreads": 0, "lwip": 0, "rootfs": 0, "misc": 0}

for rawf in rawfiles:
	dic = deepcopy(blank)
	reps1 = 0
	reps2 = 0
	with open(join("rawdata/" + size, rawf)) as f:
		prev = 0
		for line in f:
			match_time = re.match("(\d+) trace_boot_time_init.+", line)
			match_plat = re.match("(\d+) trace_boot_plat_init.+", line)
			match_pcib = re.match("(\d+) trace_boot_ctor_beg: " + functions["pci-bus"] + ".+", line)
			match_pcie = re.match("(\d+) trace_boot_ctor_end: " + functions["pci-bus"] + ".+", line)
			match_viobb = re.match("(\d+) trace_boot_ctor_beg: " + functions["virtio-bus"] + ".+", line)
			match_viobe = re.match("(\d+) trace_boot_ctor_end: " + functions["virtio-bus"] + ".+", line)
			match_viopb = re.match("(\d+) trace_boot_ctor_beg: " + functions["virtio-pci"] + ".+", line)
			match_viope = re.match("(\d+) trace_boot_ctor_end: " + functions["virtio-pci"] + ".+", line)
			match_vionb = re.match("(\d+) trace_boot_ctor_beg: " + functions["virtio-net"] + ".+", line)
			match_vione = re.match("(\d+) trace_boot_ctor_end: " + functions["virtio-net"] + ".+", line)
			match_vfscorefb = re.match("(\d+) trace_boot_ctor_beg: " + functions["vfscore-fdtable"] + ".+", line)
			match_vfscorefe = re.match("(\d+) trace_boot_ctor_end: " + functions["vfscore-fdtable"] + ".+", line)
			match_vfscoreib = re.match("(\d+) trace_boot_ctor_beg: " + functions["vfscore-init"] + ".+", line)
			match_vfscoreie = re.match("(\d+) trace_boot_ctor_end: " + functions["vfscore-init"] + ".+", line)
			match_allocb = re.match("(\d+) trace_boot_alloc_beg.+", line)
			match_alloce = re.match("(\d+) trace_boot_alloc_end.+", line)
			match_ukbusb = re.match("(\d+) trace_boot_inittab_beg: " + functions["ukbus"] + ".+", line)
			match_ukbuse = re.match("(\d+) trace_boot_inittab_end: " + functions["ukbus"] + ".+", line)
			match_pthreadb = re.match("(\d+) trace_boot_inittab_beg: " + functions["pthread-embedded"] + ".+", line)
			match_pthreade = re.match("(\d+) trace_boot_inittab_end: " + functions["pthread-embedded"] + ".+", line)
			match_lwipb = re.match("(\d+) trace_boot_inittab_beg: " + functions["lwip"] + ".+", line)
			match_lwipe = re.match("(\d+) trace_boot_inittab_end: " + functions["lwip"] + ".+", line)
			match_rootfb = re.match("(\d+) trace_boot_inittab_beg: " + functions["vfscore-rootfs"] + ".+", line)
			match_rootfe = re.match("(\d+) trace_boot_inittab_end: " + functions["vfscore-rootfs"] + ".+", line)
			match_end = re.match("(\d+) trace_boot_end.+", line)
			if (match_time):
				dic["plat"] += (int(match_time.group(1)) - prev) / 1000
				prev = int(match_time.group(1))
				reps1 += 1
			elif (match_plat):
				dic["plat"] += (int(match_plat.group(1)) - prev) / 1000
				prev = int(match_plat.group(1))
			elif (match_pcib):
				dic["misc"] += (int(match_pcib.group(1)) - prev) / 1000
				prev = int(match_pcib.group(1))
			elif (match_pcie):
				dic["virtio"] += (int(match_pcie.group(1)) - prev) / 1000
				prev = int(match_pcie.group(1))
			elif (match_viobb):
				dic["misc"] += (int(match_viobb.group(1)) - prev) / 1000
				prev = int(match_viobb.group(1))
			elif (match_viobe):
				dic["virtio"] += (int(match_viobe.group(1)) - prev) / 1000
				prev = int(match_viobe.group(1))
			elif (match_viopb):
				dic["misc"] += (int(match_viopb.group(1)) - prev) / 1000
				prev = int(match_viopb.group(1))
			elif (match_viope):
				dic["virtio"] += (int(match_viope.group(1)) - prev) / 1000
				prev = int(match_viope.group(1))
			elif (match_vionb):
				dic["misc"] += (int(match_vionb.group(1)) - prev) / 1000
				prev = int(match_vionb.group(1))
			elif (match_vione):
				dic["virtio"] += (int(match_vione.group(1)) - prev) / 1000
				prev = int(match_vione.group(1))
			elif (match_vfscorefb):
				dic["misc"] += (int(match_vfscorefb.group(1)) - prev) / 1000
				prev = int(match_vfscorefb.group(1))
			elif (match_vfscorefe):
				dic["vfscore"] += (int(match_vfscorefe.group(1)) - prev) / 1000
				prev = int(match_vfscorefe.group(1))
			elif (match_vfscoreib):
				dic["misc"] += (int(match_vfscoreib.group(1)) - prev) / 1000
				prev = int(match_vfscoreib.group(1))
			elif (match_vfscoreie):
				dic["vfscore"] += (int(match_vfscoreie.group(1)) - prev) / 1000
				prev = int(match_vfscoreie.group(1))
			elif (match_allocb):
				dic["misc"] += (int(match_allocb.group(1)) - prev) / 1000
				prev = int(match_allocb.group(1))
			elif (match_alloce):
				dic["alloc"] += (int(match_alloce.group(1)) - prev) / 1000
				prev = int(match_alloce.group(1))
			elif (match_ukbusb):
				dic["misc"] += (int(match_ukbusb.group(1)) - prev) / 1000
				prev = int(match_ukbusb.group(1))
			elif (match_ukbuse):
				dic["ukbus"] += (int(match_ukbuse.group(1)) - prev) / 1000
				prev = int(match_ukbuse.group(1))
			elif (match_pthreadb):
				dic["misc"] += (int(match_pthreadb.group(1)) - prev) / 1000
				prev = int(match_pthreadb.group(1))
			elif (match_pthreade):
				dic["pthreads"] += (int(match_pthreade.group(1)) - prev) / 1000
				prev = int(match_pthreade.group(1))
			elif (match_lwipb):
				dic["misc"] += (int(match_lwipb.group(1)) - prev) / 1000
				prev = int(match_lwipb.group(1))
			elif (match_lwipe):
				dic["lwip"] += (int(match_lwipe.group(1)) - prev) / 1000
				prev = int(match_lwipe.group(1))
			elif (match_rootfb):
				dic["misc"] += (int(match_rootfb.group(1)) - prev) / 1000
				prev = int(match_rootfb.group(1))
			elif (match_rootfe):
				dic["rootfs"] += (int(match_rootfe.group(1)) - prev) / 1000
				prev = int(match_rootfe.group(1))
			elif (match_end):
				dic["misc"] += (int(match_end.group(1)) - prev) / 1000
				prev = 0
				reps2 += 1

	if (reps1 != reps2): # sanity check
		print("sanity check error: " + rawf + " probably corrupted")
		sys.exit(1)

	match = re.match("(\w+)-(\d+).txt", rawf)
	final = "component\tboottime_us\n"
	for key in dic.keys():
		final += (key + "\t%.2f" % round(dic[key] / reps1,2)).ljust(10)
		final += "\n"

	makedirs("results", exist_ok=True)
	makedirs(join("results", size), exist_ok=True)
	with open(join(join("results", size), match.group(1) + ".csv"), "w") as f:
		f.write(final)
