#!/usr/bin/python3
import sys
import numpy

tsc_hz=0
if len(sys.argv) >= 3 and int(sys.argv[2]) > 0:
	tsc_hz=int(sys.argv[2])*1000000
else:
	tsc_hz=2600*1000000
	sys.stderr.write("WARNING: Do not have TSC frequency from experiment, assuming " + str(float(tsc_hz) / 1000000000.) + " GHz TSC clock for time conversion.\n" )

def tsc2nsec(val, hz=2600000000):
	return float(val) / (float(hz)/ 1000000000.)

data = []
data_nsec = []

# Load data in memory
with open(sys.argv[1]) as f:
   for fl in f:
      data.append(float(fl))
      data_nsec.append(tsc2nsec(float(fl), tsc_hz))

# Calculate and output median, q1 and q3
with sys.stdout as f:
    #sys.stderr.write("tsc\tnsec\n")
    f.write(str(round(numpy.average(data),2)) + "\t"
            + str(round(numpy.average(data_nsec),2))
            + "\n")
