#!/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
#
# Authors: Alexander Jung <alexander.jung@neclab.eu>
#
# Copyright (c) 2020, NEC Europe Ltd., NEC Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

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
DEFAULT = '_'

files = []
labels = []
apps = []
memstats = {}
memusage_max = 0 # maximum observed memory size
total_apps = 0
bar_colors = {
  'nginx': '#0C8828',
  'redis': '#CE1216',
  'hello': 'dimgray',
  'sqlite': '#4BA3E1'
}

labels = {
  'unikraft': 'Rhea',
  'docker': 'Docker',
  'hermitux': 'Hermitux',
  'lupine': 'Lupine',
  'osv': 'OSv',
  'rump': 'Rumprun',
  'microvm': 'Alpine Linux'
}

for f in os.listdir(RESULTSDIR):
  if f.endswith(RESULTEXT):
    index = f.replace(RESULTEXT,'')
    unikernel = index

    if unikernel not in memstats:
      memstats[unikernel] = {}

    with open(os.path.join(RESULTSDIR, f), 'r') as csvfile:
      csvdata = csv.reader(csvfile, delimiter="\t")
      
      next(csvdata) # skip header

      for row in csvdata:
        app = row[0]

        memusagemb = int(row[1]) * KBYTES * KBYTES
        memstats[unikernel][app] = memusagemb
        
        if memusagemb > memusage_max:
          memusage_max = memusagemb

# General style
common_style(plt)

memusage_max += KBYTES * KBYTES * 14 # add MB "margin"

# Setup matplotlib axis
fig = plt.figure(figsize=(8, 5))
renderer = fig.canvas.get_renderer()

# image size axis
ax1 = fig.add_subplot(1,1,1)
ax1.set_ylabel("Minimum Memory Requirement")
ax1.grid(which='major', axis='y', linestyle=':', alpha=0.5, zorder=0)
ax1_yticks = np.arange(0, memusage_max, step=KBYTES*KBYTES * 8)
ax1.set_yticks(ax1_yticks, minor=False)
ax1.set_yticklabels([sizeof_fmt(ytick) for ytick in ax1_yticks])
ax1.set_ylim(0, memusage_max)

# Plot coordinates
scale = 1. / len(memstats.keys())
xlabels = []

# Adjust margining
# fig.subplots_adjust(bottom=.) #, top=1)

i = 0
line_offset = 0
for unikernel in ['unikraft', 'docker', 'rump', 'hermitux', 'lupine', 'osv', 'microvm']:
  xlabels.append(labels[unikernel])
  apps = memstats[unikernel]

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
    app = memstats[unikernel][app_label]

    print(unikernel, app_label, app)

    bar = ax1.bar([i + 1 + bar_offset], app,
      label=app_label,
      align='center',
      zorder=3,
      width=bar_width,
      color=bar_colors[app_label],
      linewidth=.5
    )
    
    ax1.text(i + 1.02 + bar_offset, app + 1000000, sizeof_fmt(app),
      ha='center',
      va='bottom',
      fontsize=LARGE_SIZE,
      linespacing=0,
      #bbox=dict(pad=-.6, facecolor='white', linewidth=0),
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
makedirs("../plots", exist_ok=True)
fig.savefig("../plots/compare_minmem.pdf") #, bbox_extra_artists=(ax1,), bbox_inches='tight')
