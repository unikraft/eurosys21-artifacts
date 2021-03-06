#!/bin/env python3
#
# Authors: Alexander Jung <alexander.jung@neclab.eu>
#

import sys
import csv
import fire
import os.path
import matplotlib.pyplot as plt
from platform import python_version as pythonversion
from matplotlib import __version__ as matplotlibversion

def plot(data=None, output=None):
  if not os.path.isfile(data):
    print("cannot find: %s" % data)
    sys.exit(1)


  setup_labels = {
    'linux-metal': 'Linux baremetal',
    'linux-guest': 'Linux guest',
    'unikraft-guest': 'Unikraft guest'
  }

  mode_labels = {
    'single': 'Single',
    'batch': 'Batch',
    'dpdk': 'DPDK',
    'lwip': 'LwIP',
    'uknetdev': 'uknetdev'
  }

  # parsed = {}

  # Setup matplotlib axis
  fig = plt.figure(figsize=(10, 6))
  renderer = fig.canvas.get_renderer()

  # image size axis
  ax1 = fig.add_subplot(1,1,1)

  table_vals = []

  with open(data, 'r') as csvfile:
    csvdata = csv.reader(csvfile, delimiter=",")

    next(csvdata) # skip header

    for row in csvdata:
      table_vals.append([
        setup_labels[row[0]],
        mode_labels[row[1]],
        row[2]
      ])

  col_labels = ['Setup', 'Mode', 'Throughout']


  # Draw table
  table = plt.table(cellText=table_vals,
                        colWidths=[0.12, 0.10, 0.08],
                        colLabels=col_labels,
                        loc='center',
                        cellLoc='left')
  table.auto_set_font_size(False)
  table.set_fontsize(24)
  table.scale(3, 4)

  # Removing ticks and spines enables you to get the figure only with table
  plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)
  plt.tick_params(axis='y', which='both', right=False, left=False, labelleft=False)

  for pos in ['right','top','bottom','left']:
      plt.gca().spines[pos].set_visible(False)

  # Save to file
  fig.tight_layout()
  plt.savefig(output)

if __name__ == '__main__':
  fire.Fire(plot)
