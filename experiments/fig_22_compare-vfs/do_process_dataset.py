#!/usr/bin/python3
import sys
import numpy

tsc_hz=2600*1000000

def tsc2nsec(val, hz=2600000000):
	sys.stderr.write("NOTE: Assuming " + str(float(tsc_hz) / 1000000000.) + " GHz TSC clock for conversion.\n" )
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
