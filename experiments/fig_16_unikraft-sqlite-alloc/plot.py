#!/bin/env python3
#
# Authors: Alexander Jung <alexander.jung@neclab.eu>
#

import os
import csv
import sys
import numpy as np
from time import gmtime
from time import strftime
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from common import sizeof_fmt, common_style, mk_groups, KBYTES, SMALL_SIZE, MEDIUM_SIZE, LARGE_SIZE
from os import listdir, makedirs

import pprint
pp = pprint.PrettyPrinter(indent=4)

WORKDIR = os.getcwd()
RESULTSDIR = WORKDIR + '/results/'
RESULTEXT = '.csv'
GROUP_BAR_WIDTH = .8
MEAN_KEY = 'mean'
MEDIAN_KEY = 'median'
AMAX_KEY = 'amax'
AMIN_KEY = 'amin'
BASE_ALLOC = 'mimalloc'

files = []
apps = []
base_stats = {}
other_stats = {}
speedup_max = 0 # maximum observed rx mpps
bar_colors = {
  'mimalloc': '#ddcae3',
  'tinyalloc': '#ededed',
  'tlsf': '#618c84',
  'bbuddy': '#49687c'
}

labels = {
  'mimalloc': 'mimalloc',
  'tinyalloc': 'tinyalloc',
  'tlsf': 'TLSF',
  'bbuddy': 'Binary Buddy'
}

for f in os.listdir(RESULTSDIR):
  if f.endswith(RESULTEXT):
    index = f.replace(RESULTEXT,'')
    files.append(f)

    alloc = index

    with open(os.path.join(RESULTSDIR, f), 'r') as csvfile:
      csvdata = csv.reader(csvfile, delimiter="\t")
      
      next(csvdata) # skip header

      speedups = {}

      for row in csvdata:
        if int(row[0]) not in speedups:
          speedups[int(row[0])] = []

        speedups[int(row[0])].append(float(row[1]))

        if float(row[1]) > speedup_max:
          speedup_max = float(row[1])      

      for num_queries in speedups.keys():
        if alloc == BASE_ALLOC and num_queries not in base_stats:
            base_stats[num_queries] = {
              MEAN_KEY: 0,
              MEDIAN_KEY: 0,
              AMAX_KEY: 0,
              AMIN_KEY: 0,
            }

        else:
          if num_queries not in other_stats:
            other_stats[num_queries] = {
              alloc: {
                MEAN_KEY: 0,
                MEDIAN_KEY: 0,
                AMAX_KEY: 0,
                AMIN_KEY: 0,
              }
            }

        speedup_num_queries = np.array(speedups[num_queries])

        mean = float(np.average(speedup_num_queries))
        median = float(np.median(speedup_num_queries))
        amax = float(np.amax(speedup_num_queries))
        amin = float(np.amin(speedup_num_queries))

        save_stats = {
          MEAN_KEY: mean,
          MEDIAN_KEY: median,
          AMAX_KEY: amax,
          AMIN_KEY: amin,
        }

        if alloc == BASE_ALLOC:
          base_stats[num_queries] = save_stats

        else:
          other_stats[num_queries][alloc] = save_stats

# Calculate the relativity
relative_stats = {}
for num_queries in other_stats.keys():
  allocators = other_stats[num_queries]

  if num_queries not in relative_stats:
    relative_stats[num_queries] = {}

  for allocator in allocators:

    other_val = other_stats[num_queries][allocator][MEAN_KEY]
    base_val = base_stats[num_queries][MEAN_KEY]

    relative_stats[num_queries][allocator] = ((base_val - other_val)/float(other_val))*100

# General style
common_style(plt)

# speedup_max += KBYTES * KBYTES * 4 # add "margin" above tallest bar

# Setup matplotlib axis
fig = plt.figure(figsize=(8, 5))
renderer = fig.canvas.get_renderer()

# image size axis
ax1 = fig.add_subplot(1,1,1)
ax1.set_ylabel("Relative Execution Speedup")
# ax1.set_xlabel("Number of SQL INSERT Queries")

ax1.grid(which='major', axis='y', linestyle=':', alpha=0.5, zorder=0)

ax1_yticks = np.arange(-100, 50, step=20)
print(ax1_yticks)
ax1.set_yticks(ax1_yticks, minor=False)
ax1.set_yticklabels([str(ytick) for ytick in ax1_yticks])

# ax1_yticks = np.arange(0, speedup_max, step=1000000000)
# ax1.set_yticks(ax1_yticks, minor=False)
# ax1.set_yticklabels(ax1_yticks)
ax1.set_ylim(-100, 50)

# Plot coordinates
scale = 1. / len(relative_stats.keys())
xlabels = []

# Adjust margining
# fig.subplots_adjust(bottom=.15) #, top=1)



# x = range(7)
# negative_data = [-1,-4,-3,-2,-6,-2,-8]
# positive_data = [4,2,3,1,4,6,7,]

# fig = plt.figure()
# ax = plt.subplot(111)
# ax.bar(x, negative_data, width=1, color='r')
# ax.bar(x, positive_data, width=1, color='b')



i = 0
line_offset = 0
for numqueries in relative_stats.keys():

  # Plot a line beteween numqueries
  if i > 0:
    line = plt.Line2D([i * scale, i * scale], [-.02, 1],
        transform=ax1.transAxes, color='black',
        linewidth=1)
    line.set_clip_on(False)
    ax1.add_line(line)


  xlabels.append(numqueries)
  allocators = relative_stats[numqueries]

  # Plot a line beteween unikernel applications
  # if i > 0:
  #   line = plt.Line2D([i * scale, i * scale], [-.02, 1],
  #       transform=ax1.transAxes, color='black',
  #       linewidth=1)
  #   line.set_clip_on(False)
  #   ax1.add_line(line)

  j = 0
  bar_width = GROUP_BAR_WIDTH / len(allocators.keys())
  bar_offset = (bar_width / 2) - (GROUP_BAR_WIDTH / 2)

  # Plot each allocator
  for allocator_label in sorted(allocators):
    # app = stats[unikernel]


    relativity = relative_stats[numqueries][allocator_label]
    print(numqueries, allocator_label, relativity)

    bar = ax1.bar([i + 1 + bar_offset], relativity,
      label=labels[allocator_label],
      align='center',
      zorder=3,
      width=bar_width,
      color=bar_colors[allocator_label],
      linewidth=.5
    )
    
    ax1.text(i + 1 + bar_offset, relativity, ("%3.1f" % relativity),
      ha='center',
      va= 'bottom' if relativity > 0 else 'top',
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
# ax1.set_xticklabels(xlabels, fontsize=LARGE_SIZE, rotation=45, ha='right', rotation_mode='anchor')
ax1.set_xticklabels(xlabels, fontsize=LARGE_SIZE, fontweight='bold')
ax1.set_xlim(.5, len(xlabels) + .5)
ax1.yaxis.grid(True, zorder=0, linestyle=':')
ax1.tick_params(axis='both', which='both', length=0)

# Create a unique legend
handles, labels = plt.gca().get_legend_handles_labels()
by_label = dict(zip(labels, handles))
leg = plt.legend(by_label.values(), by_label.keys(),
  loc='lower left',
  ncol=1,
  fontsize=LARGE_SIZE,
)
leg.get_frame().set_linewidth(0.0)

plt.setp(ax1.lines, linewidth=.5)

# Save to file
fig.tight_layout()
makedirs("../plots", exist_ok=True)
fig.savefig("../plots/unikraft_alloc_sqlite.pdf") #, bbox_extra_artists=(ax1,), bbox_inches='tight')
