#!/usr/bin/python3
import sys
import numpy

fcall  = []
scall  = []
tsc    = []

# Load data in memory
with open(sys.argv[1] + ".fcall") as f:
   for fl in f:
        fcall.append(int(fl))

with open(sys.argv[1] + ".scall") as f:
   for fl in f:
        scall.append(int(fl))

with open(sys.argv[1] + ".tsc") as f:
   for fl in f:
        tsc.append(int(fl))

# Calculate and output median, q1 and q3
#with open(stdout, "w") as f:
with sys.stdout as f:
    f.write("\tavg\tmed\tq1\tq3\n")
    f.write("tsc\t" +  str(round(numpy.average(tsc),2)) + "\t"
			  + str(round(numpy.quantile(tsc, .50),2)) + "\t" + str(round(numpy.quantile(tsc, .25),2)) +
                 "\t" + str(round(numpy.quantile(tsc, .75),2)) + "\n")
    f.write("fcall\t" +  str(round(numpy.average(fcall),2)) + "\t"
			+ str(round(numpy.quantile(fcall, .50),2)) + "\t" + str(round(numpy.quantile(fcall, .25),2)) +
                 "\t" + str(round(numpy.quantile(fcall, .75),2)) + "\n")
    f.write("scall\t" +  str(round(numpy.average(scall),2)) + "\t"
			+ str(round(numpy.quantile(scall, .50),2)) + "\t" + str(round(numpy.quantile(scall, .25),2)) +
                 "\t" + str(round(numpy.quantile(scall, .75),2)) + "\n")
