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

def crosstick(label="X"):
  if label == "V":
    return "✓"
  else:
    return "✗"

def plot(data=None, output=None):
  if not os.path.isfile(data):
    print("cannot find: %s" % data)
    sys.exit(1)

  # platform_labels = {
  #   'linux': 'Linux/KVM',
  #   'linux-nomitig': 'Linux/KVM',
  #   'unikraft': 'Unikraft/KVM',
  #   'both': 'Both'
  # }

  # rcall_labels = {
  #   'scall': 'System call',
  #   'scall-nomitig': 'System call (no mitigations)',
  #   'fcall': 'Function Call'
  # }

  col_labels = {
    'name': 'Library',
    'musl-size': 'musl\nsize\n(MB)',
    'musl-std': 'musl\nstd.',
    'musl-compat': 'musl\ncompat.\nlayer',
    'newlib-size': 'newlib\nsize\n(MB)',
    'newlib-std': 'newlib\nstd.',
    'newlib-compat': 'newlib\ncompat.\nlayer',
    'cloc': 'Glue\nCode\nLoC'
  }

  parsed = {}
  table_vals = []

  with open(data, 'r') as csvfile:
    csvdata = csv.reader(csvfile, delimiter=",")

    next(csvdata) # skip header
  #   curr = None

    for row in csvdata:
      parsed[row[0]] = {
        'musl-size': row[1],
        'musl-std': row[2],
        'musl-compat': row[3],
        'newlib-size': row[4],
        'newlib-std': row[5],
        'newlib-compat': row[6],
        'cloc': row[7]
      }

  # Setup matplotlib axis
  fig = plt.figure(figsize=(13, 20))
  renderer = fig.canvas.get_renderer()

  # image size axis
  ax1 = fig.add_subplot(1,1,1)

  col_labels = [
    col_labels[k] for k in col_labels
  ]

  for lib in parsed:
    table_vals.append([
      lib,
      parsed[lib]['musl-size'],
      crosstick(parsed[lib]['musl-std']),
      crosstick(parsed[lib]['musl-compat']),
      parsed[lib]['newlib-size'],
      crosstick(parsed[lib]['newlib-std']),
      crosstick(parsed[lib]['newlib-compat']),
      parsed[lib]['cloc']
    ])

  # Draw table
  table = plt.table(cellText=table_vals,
                        colWidths=[0.12, 0.05, 0.04, 0.06, 0.05, 0.05, 0.06, 0.05],
                        colLabels=col_labels,
                        loc='center',
                        cellLoc='left')
  
  # set header cell height
  cellDict = table.get_celld()
  for i in range(0,len(col_labels)):
    cellDict[(0,i)].set_height(.02)
    # for j in range(1,len(table_vals)+1):
    #   cellDict[(j,i)].set_height(.01)

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
