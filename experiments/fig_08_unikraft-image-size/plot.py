#!/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
# Authors: Alexander Jung <alexander.jung@neclab.eu>

import os
import csv
import sys
import fire
import numpy as np
from time import gmtime
from time import strftime
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from common import common_style, mk_groups, SMALL_SIZE, MEDIUM_SIZE, LARGE_SIZE

def plot(data=None, output=None):
  OUTFILE = output
  RESULTEXT = '.csv'
  KBYTES = 1024.0
  IMAGESTAT = 'imagestats'
  IMAGE_SIZE_KEY = 'image_size'
  NUMSYMS_KEY = 'number_symbols'
  GROUP_BAR_WIDTH = .8
  DEFAULT = 'default'

  files = []
  labels = []
  apps = []
  imagestats = {}
  imagesize_max = 0 # maximum observed image size
  number_symbols_max = 0 # maximum observed symbol count
  total_apps = 0
  bar_colors = {}

  text_labels = {
    DEFAULT: 'Default configuration',
    'dce': '+ Dead Code Elim. (DCE)',
    'lto': '+ Link-Time Optim. (LTO)',
    'dce+lto': '+ DCE + LTO',
    'perf': '+ Performance Optimizations'
  }

  colors = sorted([
    'sandybrown',
    'teal',
    'deepskyblue',
    'lightskyblue',
    'orchid',
  ])

  def sizeof_fmt(num, suffix='B'):
    for unit in ['','K','M','G']:
      if abs(num) < KBYTES:
          return "%3.1f%s%s" % (num, unit, suffix)
      num /= KBYTES
    return "%.1f%s%s" % (num, 'Yi', suffix)

  with open(data, 'r') as csvfile:
    csvdata = csv.reader(csvfile, delimiter="\t")

    for row in csvdata:
      name = row[0].split('_', 1)
      app = name[0]
      app_type = name[1].replace('_', '+')
      
      if app not in imagestats:
        imagestats[app] = {}
        total_apps += 1
      
      if app_type not in bar_colors:
        bar_colors[app_type] = colors[1]
        colors.pop(1)

      imagestats[app][app_type] = int(row[1])


  # General style
  common_style(plt)

  imagesize_max = KBYTES * KBYTES * 3.1 # add MB "margin"
  number_symbols_max += 2000

  # Setup matplotlib axis
  fig = plt.figure(figsize=(8, 5))
  renderer = fig.canvas.get_renderer()

  # image size axis
  ax1 = fig.add_subplot(1,1,1)
  ax1.set_ylabel("Image size")
  ax1.grid(which='major', axis='y', linestyle=':', alpha=0.5)
  ax1_yticks = np.arange(0, imagesize_max, step=KBYTES*KBYTES)
  ax1.set_yticks(ax1_yticks, minor=False)
  ax1.set_yticklabels([sizeof_fmt(ytick) for ytick in ax1_yticks])
  ax1.set_ylim(0, imagesize_max)

  # Plot coordinates
  scale = 1. / total_apps
  xlabels = []

  # Adjust margining
  fig.subplots_adjust(bottom=.15) #, top=1)

  i = 0
  # Plot each application
  for app_label in sorted(imagestats):
    app = imagestats[app_label]
    xlabels.append(app_label)

    # Plot a line beteween unikernel applications
    if i > 0:
      line = plt.Line2D([i * scale, i * scale], [-.02, 1],
          transform=ax1.transAxes, color='black',
          linewidth=1)
      line.set_clip_on(False)
      ax1.add_line(line)

    j = 0
    bar_width = GROUP_BAR_WIDTH / len(app.keys())
    bar_offset = (bar_width / 2) - (GROUP_BAR_WIDTH / 2)

    for app_type in [DEFAULT, 'lto', 'dce', 'dce+lto']:
      bar = ax1.bar([i + 1 + bar_offset], app[app_type],
        label=text_labels[app_type],
        align='center',
        zorder=3,
        width=bar_width,
        color=bar_colors[app_type],
        linewidth=.5
      )
      
      ax1.text(i + 1 + bar_offset, app[app_type] + 50000, sizeof_fmt(app[app_type]),
        ha='center',
        va='bottom',
        fontsize=LARGE_SIZE,
        linespacing=0,
        bbox=dict(pad=-.6, facecolor='white', linewidth=0),
        rotation='vertical'
      )

      bar_offset += bar_width
      j += 1
    i += 1

  # set up x-axis labels
  xticks = range(1, len(xlabels) + 1)
  ax1.set_xticks(xticks)
  ax1.set_xticklabels(xlabels, fontsize=LARGE_SIZE, rotation=40, ha='right', rotation_mode='anchor')
  # ax1.set_xticklabels(xlabels, fontsize=LARGE_SIZE, fontweight='bold')
  ax1.set_xlim(.5, len(xlabels) + .5)
  ax1.yaxis.grid(True, zorder=0, linestyle=':')
  ax1.tick_params(axis='both', which='both', length=0)

  plt.setp(ax1.lines, linewidth=.5)

  # Create a unique legend
  handles, labels = plt.gca().get_legend_handles_labels()
  by_label = dict(zip(labels, handles))
  leg = plt.legend(by_label.values(), by_label.keys(), loc='upper left', ncol=2, fontsize=LARGE_SIZE*0.8)
  leg.get_frame().set_linewidth(0.0)

  # Save to file
  fig.tight_layout()
  fig.savefig(OUTFILE) #, bbox_extra_artists=(ax1,), bbox_inches='tight')

if __name__ == '__main__':
  fire.Fire(plot)