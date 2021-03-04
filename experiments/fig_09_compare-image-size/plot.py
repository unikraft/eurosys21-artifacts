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
from os import listdir, makedirs

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
  labels = []
  apps = []
  imagestats = {}
  imagesize_max = 0 # maximum observed image size
  number_symbols_max = 0 # maximum observed symbol count
  total_apps = 0
  bar_colors = {
    'nginx': '#0C8828',
    'redis': '#CE1216',
    'hello': 'dimgray',
    'sqlite': '#4BA3E1'
  }

  labels = {
    'hermitux': 'Hermitux',
    'linuxuser': 'Linux User',
    'lupine': 'Lupine',
    'osv': 'OSv',
    'rump': 'Rumprun',
    'unikraft': 'Rhea',
    'mirage': 'Mirage'
  }

  # Prepare maxplotlib data by parsing the individual .csv files.  This process
  # goes through all image sizes and number of symbols and populates a dictionary
  # of unikernels and the application "image stats" based on the framework.
  for f in os.listdir(RESULTSDIR):
    if f.endswith(RESULTEXT):
      index = f.replace(RESULTEXT,'')
      files.append(f)

      result = index.split('-')

      unikernel = result[0]
      app = result[1]

      if unikernel not in imagestats:
        imagestats[unikernel] = {}
      
      if app not in imagestats[unikernel]:
        total_apps += 1
        imagestats[unikernel][app] = 0

      if app not in apps:
        apps.append(app)

      with open(os.path.join(RESULTSDIR, f), 'r') as csvfile:
        size= int(csvfile.readline())
        imagestats[unikernel][app] = size

  # General style
  common_style(plt)

  imagesize_max += KBYTES * KBYTES * 12 # add MB "margin"
  number_symbols_max += 2000

  # Setup matplotlib axis
  fig = plt.figure(figsize=(8, 5))
  renderer = fig.canvas.get_renderer()

  # image size axis
  ax1 = fig.add_subplot(1,1,1)
  ax1.set_ylabel("Image size")
  ax1.grid(which='major', axis='y', linestyle=':', alpha=0.5, zorder=0)
  ax1_yticks = np.arange(0, imagesize_max, step=KBYTES*KBYTES*2)
  ax1.set_yticks(ax1_yticks, minor=False)
  ax1.set_yticklabels([sizeof_fmt(ytick) for ytick in ax1_yticks])
  ax1.set_ylim(0, imagesize_max)

  # Plot coordinates
  scale = 1. / len(imagestats.keys())
  xlabels = []

  # Adjust margining
  fig.subplots_adjust(bottom=.15) #, top=1)

  i = 0
  line_offset = 0
  for unikernel in [
      'unikraft',
      'hermitux',
      'linuxuser',
      'lupine',
      'mirage',
      'osv',
      'rump'
    ]:
    xlabels.append(labels[unikernel])
    apps = imagestats[unikernel]

    # Plot a line beteween unikernel applications
    if i > 0:
      line = plt.Line2D([i * scale, i * scale], [-.02, 1],
          transform=ax1.transAxes, color='black',
          linewidth=1)
      line.set_clip_on(False)
      ax1.add_line(line)

    j = 0
    bar_width = GROUP_BAR_WIDTH / len(apps.keys())
    bar_offset = (bar_width / 2) - (GROUP_BAR_WIDTH / 2)

    # Plot each application
    for app_label in sorted(apps):
      app = imagestats[unikernel][app_label]

      print(unikernel, app_label, app)

      bar = ax1.bar([i + 1 + bar_offset], app,
        label=app_label,
        align='center',
        zorder=3,
        width=bar_width,
        color=bar_colors[app_label],
        linewidth=.5
      )
      
      ax1.text(i + 1 + bar_offset, app + 500000, sizeof_fmt(app),
        ha='center',
        va='bottom',
        fontsize=LARGE_SIZE,
        linespacing=0,
        zorder=2,
        bbox=dict(pad=0, facecolor='white', linewidth=0),
        rotation='vertical'
      )

      bar_offset += bar_width
      j += 1

    i += 1

  # sys.exit(1)

  # set up x-axis labels
  xticks = range(1, len(xlabels) + 1)
  ax1.set_xticks(xticks)
  ax1.set_xticklabels(xlabels, fontsize=LARGE_SIZE, rotation=40, ha='right', rotation_mode='anchor')
  # ax1.set_xticklabels(xlabels, fontsize=LARGE_SIZE, fontweight='bold')
  ax1.set_xlim(.5, len(xlabels) + .5)
  ax1.yaxis.grid(True, zorder=0, linestyle=':')
  ax1.tick_params(axis='both', which='both', length=0)

  # Create a unique legend
  handles, labels = plt.gca().get_legend_handles_labels()
  by_label = dict(zip(labels, handles))
  leg = plt.legend(by_label.values(), by_label.keys(),
    loc='upper left',
    ncol=2,
    fontsize=LARGE_SIZE,
  )
  leg.get_frame().set_linewidth(0.0)

  plt.setp(ax1.lines, linewidth=.5)

  # Save to file
  fig.tight_layout()
  fig.savefig(output) #, bbox_extra_artists=(ax1,), bbox_inches='tight')

if __name__ == '__main__':
  fire.Fire(plot)
