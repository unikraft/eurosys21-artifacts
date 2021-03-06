#!/bin/env python3
#
# Authors: Alexander Jung <alexander.jung@neclab.eu>
#
import os
import csv
import fire
import sys
import pprint
import numpy as np
import collections
from time import gmtime
from time import strftime
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

SMALL_SIZE = 12
MEDIUM_SIZE = 14
LARGE_SIZE = 18
BIGGER_SIZE = 24

pp = pprint.PrettyPrinter(indent=4)

def common_style(plt):
  plt.style.use('classic')
  plt.tight_layout()

  plt.rcParams['text.usetex'] = False
  plt.rc('pdf', fonttype=42)

  plt.rc('font',**{
    'family':'sans-serif',
    'sans-serif':['Helvetica']}
  )
  plt.rc('text', usetex=True)

  plt.rcParams['font.family'] = "sans-serif"

  plt.rc('font', size=MEDIUM_SIZE)         # controls default text sizes
  plt.rc('axes', titlesize=MEDIUM_SIZE)    # fontsize of the axes title
  plt.rc('axes', labelsize=LARGE_SIZE)     # fontsize of the x and y labels
  plt.rc('xtick', labelsize=LARGE_SIZE)   # fontsize of the tick labels
  plt.rc('ytick', labelsize=MEDIUM_SIZE)   # fontsize of the tick labels
  plt.rc('legend', fontsize=MEDIUM_SIZE)   # legend fontsize


def plot(data=None, output=None):
  WORKDIR = os.getcwd()
  GRAPHSDIR = WORKDIR + ''
  RESULTSDIR = data
  RESULTEXT = '.csv'
  GROUP_BAR_WIDTH = .8
  DEFAULT = '_'

  files = []
  apps = []
  stats = {}
  delay_max = 0 # maximum observed rx mpps
  bar_colors = {
    'unikraft-kvm_read': '#0B5DA2',
    'unikraft-kvm_write': '#000000',
    'linux-kvm_read': '#DC000F',
    'linux-kvm_write': '#8000CA'
  }
  markers = {
    'unikraft-kvm_read': 'x',
    'unikraft-kvm_write': ',',
    'linux-kvm_read': '.',
    'linux-kvm_write': '4'
  }

  labels = {
    'unikraft-kvm_read': 'unikraft-kvm (read)',
    'unikraft-kvm_write': 'unikraft-kvm (write)',
    'linux-kvm_read': 'linux-kvm (read)',
    'linux-kvm_write': 'linux-kvm (write)'
  }

  for f in os.listdir(RESULTSDIR):
    if f.endswith(RESULTEXT):
      index = f.replace(RESULTEXT,'')
      files.append(f)

      unikernel = index

      with open(os.path.join(RESULTSDIR, f), 'r') as csvfile:
        csvdata = csv.reader(csvfile, delimiter=",")
        
        next(csvdata) # skip header

        for row in csvdata:
          if unikernel not in stats:
            stats[unikernel] = {}
          
          delay = float(row[1])
          stats[unikernel][str(row[0])] = delay

          if delay > delay_max:
            delay_max = delay


  # General style
  common_style(plt)

  delay_max += 2000

  # Setup matplotlib axis
  fig = plt.figure(figsize=(8, 5))
  renderer = fig.canvas.get_renderer()

  # image size axis
  ax1 = fig.add_subplot(1,1,1)
  ax1.set_ylabel("Latency (microseconds)")
  ax1.set_xlabel("Block Size (KB)")
  ax1.grid(which='major', axis='y', linestyle=':', alpha=0.5, zorder=0)
  ax1.set_yscale('log')

  ax1_yticks = np.array([1,10,100,1000, 10000, 60000])
  ax1.set_yticks(ax1_yticks, minor=False)
  ax1.set_yticklabels([ytick for ytick in ax1_yticks])
  ax1.set_ylim(1, delay_max)

  # Plot coordinates
  xlabels = list(stats[list(stats.keys())[0]].keys())

  # Adjust margining
  fig.subplots_adjust(bottom=.15) #, top=1)

  for unikernel in sorted(stats.keys()):
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
  leg = plt.legend(by_label.values(), by_label.keys(), fontsize=LARGE_SIZE, loc='best', ncol=2)
  leg.get_frame().set_linewidth(0.0)

  # Save to file
  fig.tight_layout()
  fig.savefig(output)

if __name__ == '__main__':
  fire.Fire(plot)
