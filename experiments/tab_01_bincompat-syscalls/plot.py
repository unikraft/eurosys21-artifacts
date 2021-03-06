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

  platform_labels = {
    'linux': 'Linux/KVM',
    'linux-nomitig': 'Linux/KVM',
    'unikraft': 'Unikraft/KVM',
    'both': 'Both'
  }

  rcall_labels = {
    'scall': 'System call',
    'scall-nomitig': 'System call (no mitigations)',
    'fcall': 'Function Call'
  }

  parsed = {}
  table_vals2 = []

  with open(data, 'r') as csvfile:
    csvdata = csv.reader(csvfile, delimiter=",")

    curr = None

    for row in csvdata:
      if row[0] not in parsed:
        parsed[row[0]] = {}
      
      parsed[row[0]][row[1]] = row[3]

  print(parsed)

  # Setup matplotlib axis
  fig = plt.figure(figsize=(12, 3))
  renderer = fig.canvas.get_renderer()

  # image size axis
  ax1 = fig.add_subplot(1,1,1)

  col_labels = ['Platform', 'Routine call', '#Cycles', 'nsecs']

  table_vals = [
    [platform_labels['linux'],         rcall_labels['scall'],         parsed['linux']['scall'],         parsed['linux']['scall_nsec']        ],
    [platform_labels['linux-nomitig'], rcall_labels['scall-nomitig'], parsed['linux-nomitig']['scall'], parsed['linux-nomitig']['scall_nsec']],
    [platform_labels['unikraft'],      rcall_labels['scall'],         parsed['unikraft']['scall'],      parsed['unikraft']['scall_nsec']     ],
    [platform_labels['both'],          rcall_labels['fcall'],         parsed['linux']['fcall'],         parsed['linux']['fcall_nsec']        ]
  ]

  # Draw table
  table = plt.table(cellText=table_vals,
                        colWidths=[0.09, 0.17, 0.06, 0.06],
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
