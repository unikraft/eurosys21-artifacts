#!/bin/env python3
#
# Authors: Alexander Jung <alexander.jung@neclab.eu>
#

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

import pprint
pp = pprint.PrettyPrinter(indent=4)

def plot(data=None, output=None):
  WORKDIR = os.getcwd()
  RESULTSDIR = data
  RESULTEXT = '.csv'
  IMAGESTAT = 'imagestats'
  IMAGE_SIZE_KEY = 'image_size'
  NUMSYMS_KEY = 'number_symbols'
  GROUP_BAR_WIDTH = .8
  DEFAULT = '_'

  files = []
  apps = []
  stats = {}
  throughput_max = 0 # maximum observed rx mpps
  bar_colors = {
    'linux-dpdk-vhost-user': '#0B5DA2',
    'linux-dpdk-vhost-net': '#000000',
    'unikraft-vhost-user': '#DC000F',
    'unikraft-vhost-net': '#8000CA'
  }
  markers = {
    'linux-dpdk-vhost-user': 'x',
    'linux-dpdk-vhost-net': ',',
    'unikraft-vhost-user': '.',
    'unikraft-vhost-net': '4'
  }

  labels = {
    'linux-dpdk-vhost-user': 'Linux DPDK with vhost-user',
    'linux-dpdk-vhost-net': 'Linux DPDK with vhost-net',
    'unikraft-vhost-user': 'Rhea with vhost-user',
    'unikraft-vhost-net': 'Rhea with vhost-net'
  }

  for f in os.listdir(RESULTSDIR):
    if f.endswith(RESULTEXT):
      index = f.replace(RESULTEXT,'')
      files.append(f)

      unikernel = index

      with open(os.path.join(RESULTSDIR, f), 'r') as csvfile:
        csvdata = csv.reader(csvfile, delimiter="\t")
        
        next(csvdata) # skip header

        for row in csvdata:
          if unikernel not in stats:
            stats[unikernel] = {}
          
          throughput = float(row[1]) * KBYTES * KBYTES
          stats[unikernel][str(row[0])] = throughput

          if throughput > throughput_max:
            throughput_max = throughput

  # General style
  common_style(plt)

  throughput_max += KBYTES * KBYTES * 1 # add "margin" above tallest bar

  # Setup matplotlib axis
  fig = plt.figure(figsize=(8, 5))
  renderer = fig.canvas.get_renderer()

  # image size axis
  ax1 = fig.add_subplot(1,1,1)
  ax1.set_ylabel("Throughout (Mp/s)")
  ax1.set_xlabel("Packet Size (Bytes)")
  ax1.grid(which='major', axis='y', linestyle=':', alpha=0.5, zorder=0)
  ax1_yticks = np.arange(0, throughput_max, step=KBYTES * KBYTES * 2)
  ax1.set_yticks(ax1_yticks, minor=False)
  ax1.set_yticklabels([sizeof_fmt(ytick, suffix='') for ytick in ax1_yticks])
  ax1.set_ylim(0, throughput_max)

  # Plot coordinates
  xlabels = list(stats[list(stats.keys())[0]].keys())

  # Adjust margining
  fig.subplots_adjust(bottom=.15) #, top=1)

  for unikernel in stats.keys():
    ax1.plot(list(stats[unikernel].keys()), list(stats[unikernel].values()),
      marker=markers[unikernel],
      label=labels[unikernel],
      zorder=3,
      linewidth=3,
      markersize=9,
      markeredgewidth=4,
      color=bar_colors[unikernel],
    )

  # set up x-axis labels
  xticks = range(0, len(xlabels))
  ax1.set_xticks(xticks)

  ax1.margins(x=.05)

  # Create a unique legend
  handles, labels = plt.gca().get_legend_handles_labels()
  by_label = dict(zip(labels, handles))
  leg = plt.legend(by_label.values(), by_label.keys(), fontsize=LARGE_SIZE, loc='upper right', ncol=1)
  leg.get_frame().set_linewidth(0.0)

  # Save to file
  fig.tight_layout()
  fig.savefig(output) #, bbox_extra_artists=(ax1,), bbox_inches='tight')

if __name__ == '__main__':
  fire.Fire(plot)