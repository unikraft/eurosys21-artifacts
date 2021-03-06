#!/usr/bin/python3
import os
import sys
import numpy

name=None
tsc_hz=0
basename = sys.argv[1].split('/')[-1]

if len(sys.argv) >= 3 and int(sys.argv[2]) > 0:
	tsc_hz=int(sys.argv[2])*1000000
else:
	tsc_hz=2600*1000000
	sys.stderr.write("WARNING: Do not have TSC frequency from experiment, assuming " + str(float(tsc_hz) / 1000000000.) + " GHz TSC clock for time conversion.\n" )

def tsc2nsec(val, hz=2600000000):
	return float(val) / (float(hz)/ 1000000000.)

fcall       = []
fcall_nsec  = []
scall       = []
scall_nsec  = []
tsc         = []
tsc_nsec    = []

# Load data in memory
with open(sys.argv[1] + ".fcall") as f:
   for fl in f:
        fcall.append(int(fl))
        fcall_nsec.append(tsc2nsec(int(fl), tsc_hz))

with open(sys.argv[1] + ".scall") as f:
   for fl in f:
        scall.append(int(fl))
        scall_nsec.append(tsc2nsec(int(fl), tsc_hz))

with open(sys.argv[1] + ".tsc") as f:
   for fl in f:
        tsc.append(int(fl))
        tsc_nsec.append(tsc2nsec(int(fl), tsc_hz))

# Calculate and output median, q1 and q3
#with open(stdout, "w") as f:
with sys.stdout as f:
    f.write(basename + ",tsc," +  str(round(numpy.average(tsc),2)) + ","
			  + str(round(numpy.quantile(tsc, .50),2)) + "," + str(round(numpy.quantile(tsc, .25),2)) +
                 "," + str(round(numpy.quantile(tsc, .75),2)) + "\n")
    f.write(basename + ",tsc_nsec," +  str(round(numpy.average(tsc_nsec),2)) + ","
			  + str(round(numpy.quantile(tsc_nsec, .50),2)) + "," + str(round(numpy.quantile(tsc_nsec, .25),2)) +
                 "," + str(round(numpy.quantile(tsc_nsec, .75),2)) + "\n")
    f.write(basename + ",fcall,," +  str(round(numpy.average(fcall),2)) + ","
			+ str(round(numpy.quantile(fcall, .50),2)) + "," + str(round(numpy.quantile(fcall, .25),2)) +
                 "," + str(round(numpy.quantile(fcall, .75),2)) + "\n")
    f.write(basename + ",fcall_nsec," +  str(round(numpy.average(fcall_nsec),2)) + ","
			+ str(round(numpy.quantile(fcall_nsec, .50),2)) + "," + str(round(numpy.quantile(fcall_nsec, .25),2)) +
                 "," + str(round(numpy.quantile(fcall_nsec, .75),2)) + "\n")
    f.write(basename + ",scall,," +  str(round(numpy.average(scall),2)) + ","
			+ str(round(numpy.quantile(scall, .50),2)) + "," + str(round(numpy.quantile(scall, .25),2)) +
                 "," + str(round(numpy.quantile(scall, .75),2)) + "\n")
    f.write(basename + ",scall_nsec," +  str(round(numpy.average(scall_nsec),2)) + ","
			+ str(round(numpy.quantile(scall_nsec, .50),2)) + "," + str(round(numpy.quantile(scall_nsec, .25),2)) +
                 "," + str(round(numpy.quantile(scall_nsec, .75),2)) + "\n")
