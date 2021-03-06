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

  labels = {
    'unikraft-qemu': 'Unikraft KVM',
    'docker': 'Docker Native',
    'hermitux-uhyve': 'Hermitux uHyve',
    'osv-qemu': 'OSv KVM',
    'rump-qemu': 'Rump KVM',
    'microvm-qemu': 'Linux KVM',
    'lupine-qemu': 'Lupine KVM',
    'lupine-fc': 'Lupine FC',
    'native': 'Linux Native',
    'microvm-fc': 'Linux FC',
    'mirage-solo5': 'Mirage Solo5'
  }

  for f in os.listdir(RESULTSDIR):
    if f.endswith(RESULTEXT):
      unikernel = f.replace(RESULTEXT,'')

      if unikernel not in stats:
        stats[unikernel] = {
          MEAN_KEY: 0,
          MEDIAN_KEY: 0,
          AMAX_KEY: 0,
          AMIN_KEY: 0
        }
      
      with open(os.path.join(RESULTSDIR, f), 'r') as csvfile:
        csvdata = csv.reader(csvfile, delimiter="\t")
        
        next(csvdata) # skip header

        throughput = []

        for row in csvdata:
          throughput.append(float(row[0]) / 1000)
      
        throughput = np.array(throughput)
        throughput = {
          MEAN_KEY: np.average(throughput),
          MEDIAN_KEY: np.median(throughput),
          AMAX_KEY: np.amax(throughput),
          AMIN_KEY: np.amin(throughput)
        }

        if throughput[AMAX_KEY] > throughput_max:
          throughput_max = throughput[AMAX_KEY]

        stats[unikernel] = throughput

  # General style
  common_style(plt)

  throughput_max += 100 # margin above biggest bar

  # Setup matplotlib axis
  fig = plt.figure(figsize=(8, 5))
  renderer = fig.canvas.get_renderer()

  # image size axis
  ax1 = fig.add_subplot(1,1,1)
  ax1.set_ylabel("Average Throughput (x1000 req/s)")
  ax1.grid(which='major', axis='y', linestyle=':', alpha=0.5, zorder=0)
  ax1_yticks = np.arange(0, throughput_max, step=50)
  ax1.set_yticks(ax1_yticks, minor=False)
  ax1.set_yticklabels(["%3.0f" % ytick for ytick in ax1_yticks])
  ax1.set_ylim(0, throughput_max)

  # Plot coordinates
  scale = 1. / len(stats.keys())
  xlabels = []

  # Adjust margining
  # fig.subplots_adjust(bottom=.15) #, top=1)

  i = 0
  line_offset = 0
  for unikernel in [
    'mirage-solo5',
    'microvm-fc', 
    'lupine-fc', 
    'microvm-qemu', 
    'rump-qemu',
    'docker',
    'native',
    'lupine-qemu',
    'osv-qemu',
    'unikraft-qemu'
    ]:
    xlabels.append(labels[unikernel])
    throughput = stats[unikernel]
    yerr = throughput[AMAX_KEY]-throughput[AMIN_KEY]

    print(unikernel, throughput[MEAN_KEY], '+/-', yerr)

    # Plot each application
    bar = ax1.bar([i + 1], throughput[MEAN_KEY],
      label=unikernel,
      align='center',
      zorder=4,
      yerr=yerr,
      width=GROUP_BAR_WIDTH,
      color=bar_color,
      linewidth=.5
    )
    
    ax1.text(i + 1, throughput[MEAN_KEY] + yerr + 15, "%3.1f" % throughput[MEAN_KEY],
      ha='center',
      va='bottom',
      zorder=6,
      fontsize=LARGE_SIZE,
      linespacing=0,
      bbox=dict(pad=-.6, facecolor='white', linewidth=0),
      rotation='vertical'
    )

    i += 1

  # sys.exit(1)

  # set up x-axis labels
  xticks = range(1, len(xlabels) + 1)
  ax1.set_xticks(xticks)
  ax1.set_xticklabels(xlabels, fontsize=LARGE_SIZE, rotation=45, ha='right', rotation_mode='anchor')
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
  plt.show()
  fig.savefig(output) #, bbox_extra_artists=(ax1,), bbox_inches='tight')

if __name__ == '__main__':
  fire.Fire(plot)
