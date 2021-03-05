#!/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
#
# Authors: Alexander Jung <alexander.jung@neclab.eu>
#s

import os
import csv
import sys
import fire
import numpy as np
from time import gmtime
from time import strftime
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from common import sizeof_fmt, common_style, mk_groups, KBYTES, SMALL_SIZE, MEDIUM_SIZE, LARGE_SIZE
from os import listdir, makedirs

import pprint
pp = pprint.PrettyPrinter(indent=4)

def plot(data=None, output=None):
  WORKDIR = os.getcwd()
  RESULTSDIR = data
  RESULTEXT = '.csv'
  GROUP_BAR_WIDTH = .4
  DEFAULT = '_'
  THROUGHPUT = 'throughput'
  MEAN_KEY = 'mean'
  MEDIAN_KEY = 'median'
  AMAX_KEY = 'amax'
  AMIN_KEY = 'amin'

  files = []
  labels = []
  apps = []
  stats = {}
  throughput_max = 0 # maximum observed throughput
  total_apps = 0
  bar_color = '#5697C4'

  for f in os.listdir(RESULTSDIR):
    if f.endswith(RESULTEXT):
      alloc = f.replace(RESULTEXT,'')
      
      if alloc not in stats:
        stats[alloc] = {}
      
      with open(os.path.join(RESULTSDIR, f), 'r') as csvfile:
        csvdata = csv.reader(csvfile, delimiter="\t")
        
        next(csvdata) # skip header

        operations = {}

        for row in csvdata:
          if int(row[0]) not in operations:
            operations[int(row[0])] = []
          
          operations[int(row[0])].append(float(row[1]))
        
        for operation in operations:
          all_ops = np.array(operations[int(operation)])
          operations[int(operation)] = {
            MEAN_KEY: np.average(all_ops),
            MEDIAN_KEY: np.median(all_ops),
            AMAX_KEY: np.amax(all_ops),
            AMIN_KEY: np.amin(all_ops)
          }

          if int(round((np.amax(all_ops)))) > throughput_max:
            throughput_max = int(round((np.amax(all_ops))))

        stats[alloc] = operations

  # General style
  common_style(plt)

  throughput_max += 100 # margin above biggest bar

  # Setup matplotlib axis
  fig = plt.figure(figsize=(8, 5))
  renderer = fig.canvas.get_renderer()

  # image size axis
  ax1 = fig.add_subplot(1,1,1)
  ax1.set_ylabel("Boot Time in Microseconds")
  ax1.grid(which='major', axis='y', linestyle=':', alpha=0.5, zorder=0)
  ax1_yticks = np.arange(0, throughput_max, step=50)
  ax1.set_yticks(ax1_yticks, minor=False)
  ax1.set_yticklabels(ax1_yticks)
  ax1.set_ylim(0, throughput_max)

  # Plot coordinates
  scale = 1. / len(stats.keys())
  xlabels = []

  # Adjust margining
  # fig.subplots_adjust(bottom=.15) #, top=1)

  i = 0
  line_offset = 0
  for method in ['static', 'dynamic']:
    operations = stats[method]

    # Plot each operation
    for operation in sorted(operations):

      label = '%s %s' % (method, sizeof_fmt(operation * KBYTES * KBYTES))

      xlabels.append(label)

      print(label, operations[operation][MEAN_KEY])

      # Plot each application
      bar = ax1.bar([i + 1], operations[operation][MEAN_KEY],
        label=label,
        align='center',
        zorder=4,
        yerr=operations[operation][AMAX_KEY]-operations[operation][AMIN_KEY],
        error_kw=dict(lw=2, capsize=10, capthick=1),
        width=GROUP_BAR_WIDTH,
        color=bar_color,
        linewidth=.5
      )
    
      ax1.text(i + 1, operations[operation][AMAX_KEY] + 10,
        int(round(operations[operation][MEAN_KEY])),
        ha='center',
        va='bottom',
        zorder=6,
        fontsize=LARGE_SIZE,
        linespacing=0,
        bbox=dict(pad=-.6, facecolor='white', linewidth=0),
        rotation='vertical'
      )

      i += 1

  # set up x-axis labels
  xticks = range(1, len(xlabels) + 1)
  ax1.set_xticks(xticks)
  ax1.set_xticklabels(xlabels, fontsize=LARGE_SIZE, rotation=40, ha='right', rotation_mode='anchor')
  # ax1.set_xticklabels(xlabels, fontsize=LARGE_SIZE, fontweight='bold')
  ax1.set_xlim(.5, len(xlabels) + .5)
  ax1.yaxis.grid(True, zorder=0, linestyle=':')
  ax1.tick_params(axis='both', which='both', length=0)

  # Create a unique legend
  # handles, labels = plt.gca().get_legend_handles_labels()
  # by_label = dict(zip(labels, handles))
  # leg = plt.legend(by_label.values(), by_label.keys(), loc='upper left', ncol=4)
  # leg.get_frame().set_linewidth(0.0)

  plt.setp(ax1.lines, linewidth=.5)

  # Save to file
  fig.tight_layout()
  fig.savefig(output) #, bbox_extra_artists=(ax1,), bbox_inches='tight')

if __name__ == '__main__':
  fire.Fire(plot)