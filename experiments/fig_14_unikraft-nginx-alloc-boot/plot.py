#!/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
#
# Authors: Alexander Jung <alexander.jung@neclab.eu>
#

import os
import sys
import csv
import fire
import pprint
import numpy as np
from time import gmtime
from time import strftime
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from common import common_style, mk_groups, SMALL_SIZE, MEDIUM_SIZE, LARGE_SIZE, PATTERNS
from os import listdir, makedirs

pp = pprint.PrettyPrinter(indent=4)

def plot(data=None, output=None):
  RESULTSDIR = data
  RESULTEXT = '.csv'

  MINUTES = 60
  DEFAULT_COMPONENET_KEY = '_'
  MEAN_KEY = 'mean'
  MEDIAN_KEY = 'median'
  AMAX_KEY = 'amax'
  AMIN_KEY = 'amin'
  BAR_WIDTH = 0.6

  files = []
  labels = []
  apps = []
  boottimes = {}
  boottime_max = 0 # maximum observed build time
  stack_max = 1 # number of bars to be stacked
  total_allocators = 0
  component_colors = {
  }
  text_xlabels = {
    'buddy': 'Binary Buddy',
    'mimalloc': 'mimalloc',
    'region': 'bootalloc',
    'tinyalloc': 'tinyalloc',
    'tlsf': 'TLSF'
  }

  colors = [
    # 'black',
    # 'dimgray',
    # 'lightcoral',
    # 'orangered',
    # 'sandybrown',
    # 'darkorange',
    # 'gold',
    # 'darkkhaki',
    # 'yellowgreen',
    # 'seagreen',
    # 'turquoise',
    # 'teal',
    # 'deepskyblue',
    # 'royalblue',
    # 'mediumpurple',
    # 'orchid',
    # 'lightskyblue',

    '#91c6e7',  # blue
    '#d18282',  # red
    '#ddcae3',  # lavender
    '#a2d9d1',  # thyme
    '#ededed',  # gray
    '#fff3cd',  # yellow
    '#91c6e7',  # light blue
    '#618c84',  # dark green
    '#49687c',  # dark blue 
    '#7c4f4f',  # dark yellow
  ]

  for f in os.listdir(RESULTSDIR):
    if f.endswith(RESULTEXT):
      allocator = f.replace(RESULTEXT,'')
      files.append(f)

      # component = None

      # if '+' in unikernel:
      #   unikernel = unikernel.split('+')
      #   component = unikernel[1]
      #   unikernel = unikernel[0]


      if allocator not in boottimes:
        total_allocators += 1
        boottimes[allocator] = {}

      with open(os.path.join(RESULTSDIR, f), 'r') as csvfile:
        csvdata = csv.reader(csvfile, delimiter="\t")
        
        next(csvdata) # skip header
        
        execution_times = {}

        for row in csvdata:
          if row[0] not in execution_times:
            execution_times[row[0]] = []

          execution_times[row[0]].append(float(row[1]) / 1000.0)
        
        for component in execution_times.keys():

          # Create a unique pair for the component for an associated colour
          if component not in component_colors.keys():
            component_colors[component] = colors[1]
            colors.pop(1)

          componenet_exec_times = np.array(execution_times[component])

          mean = float(np.average(componenet_exec_times))
          median = float(np.median(componenet_exec_times))
          amax = float(np.amax(componenet_exec_times))
          amin = float(np.amin(componenet_exec_times))

          if amax > boottime_max:
            boottime_max = amax

          boottimes[allocator][component] = {
            MEAN_KEY: mean,
            MEDIAN_KEY: median,
            AMAX_KEY: amax,
            AMIN_KEY: amin,
          }

          if len(boottimes[allocator]) > stack_max:
            stack_max = len(boottimes[allocator])

  # General style
  common_style(plt)

  boottime_max += 1.5 # margin above biggest bar

  # Setup matplotlib
  fig = plt.figure(figsize=(8, 5))
  ax = fig.add_subplot(1,1,1)
  ax.grid(which='major', axis='y', linestyle=':', alpha=0.5)

  # This plot:
  # ax.set_title('Unikernel Build Times', pad=35)
  ax.set_ylabel("Total Boot Time (ms)")
  # ax.set_xlabel('Applications', labelpad=10)

  # Add padding above tallest bar

  plt.ylim(0, boottime_max)

  renderer = fig.canvas.get_renderer()

  ax_yticks = np.arange(0, boottime_max, step=0.5)
  ax.set_yticklabels([str(ytick) for ytick in ax_yticks])
  ax.set_yticks(ax_yticks, minor=False)

  # ax.set_yscale('symlog')
  # ax.set_yticks(np.arange(0, (boottime_max / MINUTES) + 10, step=2), minor=False)

  # Adjust margining
  # fig.subplots_adjust(bottom=.1) #, top=1)

  # Plot coordinates
  yticks = 0
  scale = 1. / len(text_xlabels)
  xlabels = []

  # Create a blank matrix where we'll align bar sizes for matplotlib
  means = np.zeros((stack_max, total_allocators), dict)
  labels = np.zeros((stack_max, total_allocators), dict)


  i = 0
  for allocator in text_xlabels.keys():
    # Write unikernel project on top as "header"
    lxpos = (i + .5 * len(boottimes[allocator].keys())) * scale
    xlabels.append(text_xlabels[allocator])

    # ax.text(lxpos, 1.04, r'\textbf{%s}' % unikernel, ha='center', transform=ax.transAxes, fontweight='bold')

    # # Plot a line beteween unikernel applications
    # if i > 0:
    #   line = plt.Line2D([i * scale, i * scale], [0, 1.02],
    #       transform=ax.transAxes, color='black',
    #       linewidth=1)
    #   line.set_clip_on(False)
    #   ax.add_line(line)

    components = list(boottimes[allocator].items())
    total_time = 0.

    # Plot each allocator's as a multi-bar
    j = 0
    for component_label in sorted(boottimes[allocator]):
      component = boottimes[allocator][component_label]

      means[j][i] = (component[MEAN_KEY])
      total_time += component[MEAN_KEY]
      bottom_offset = 0

      # Increase y-axis distance for the component's bar
      for k in range(j, 0, -1):
        bottom_offset += means[k - 1][i]

      # Save the component label
      if component_label == DEFAULT_COMPONENET_KEY:
        component_label = DEFAULT_COMPONENET_KEY

      if component_label == DEFAULT_COMPONENET_KEY:
        labels[j][i] = (DEFAULT_COMPONENET_KEY)
      else:
        labels[j][i] = (component_label)
      
      # Plot the bar at the correct matrix location
      bar = ax.bar([i + 1], component[MEAN_KEY],
        bottom=bottom_offset,
        label=component_label,
        align='center',
        zorder=12,
        width=BAR_WIDTH,
        color=component_colors[component_label],
        linewidth=.5
      )

      # Write total time label if last bar
      if j == len(components) - 1:
        bottom_offset += component[MEAN_KEY]  # + .28 # + spacing
        print_total_time = round(total_time, 2)

        # if total_time < 1:

        # elif total_time < MINUTES:
        #   print_total_time = "%-.2fs" % (total_time)

        # elif total_time > MINUTES * MINUTES:
        #   print_total_time = strftime("%-Hh %-Mm", gmtime(total_time))

        # else:
        #   print_total_time = strftime("%-Mm %-Ss", gmtime(total_time))
        
        plt.text(i + 1, bottom_offset + 0.1, print_total_time,
          ha='center',
          va='bottom',
          fontsize=LARGE_SIZE,
          linespacing=0,
          bbox=dict(pad=-.6, facecolor='white', linewidth=0),
          rotation='vertical'
        )
      
      # add a time label for the application
      # if len(components) > 1 and component_label == DEFAULT_COMPONENET_KEY:
      #   component_seconds = component[MEAN_KEY]
        
      #   if component_seconds < 1:
      #     print_total_time = "%-.0fms" % (component_seconds * 1000)

      #   elif component_seconds < MINUTES:
      #     print_total_time = "%-.2fs" % (component_seconds)

      #   else:
      #     print_total_time = strftime("%-Mm%-Ss", gmtime(component_seconds))

      #   print(component_seconds, print_total_time)
        
      #   # Account for very  tiny applciation builds and position above axis bar
      #   yplot = bottom_offset + component[MEAN_KEY]
      #   plt.text(i + 1, yplot, r'\textbf{%s}' % print_total_time,
      #     ha='center',
      #     va='top' if round(yplot) >= 1 else 'bottom',
      #     fontsize=LARGE_SIZE,
      #     fontweight='bold',
      #     color='white',
      #     zorder=6,
      #     bbox=dict(pad=2, facecolor='none', linewidth=0),
      #     rotation='vertical'
      #   )

      j += 1
    
    i += 1

  xticks = range(1, total_allocators + 1)

  ax.set_xticks(xticks)
  ax.set_xticklabels(xlabels, fontsize=LARGE_SIZE)
  # ax.set_xticklabels(xlabels, fontsize=LARGE_SIZE, rotation=45, ha='right', rotation_mode='anchor')
  ax.set_xlim(.5, total_allocators + .5)
  ax.yaxis.grid(True, zorder=0, linestyle=':')
  plt.setp(ax.lines, linewidth=.5)

  # Resize plot for bottom legend
  chartBox = ax.get_position()
  # ax.set_position([chartBox.x0, chartBox.y0 + chartBox.height*0.18, chartBox.width, chartBox.height*0.82])

  # Create a unique legend
  handles, labels = plt.gca().get_legend_handles_labels()
  # make sure it is in the same order as the layers
  by_label = dict(zip(labels[::-1], handles[::-1]))
  leg = plt.legend(by_label.values(), by_label.keys(),
    loc='upper right',
    ncol=3,
    fontsize=LARGE_SIZE,
    columnspacing=.8
  )
  leg.get_frame().set_linewidth(0.0)

  # Save to file
  fig.tight_layout()
  fig.savefig(output)
