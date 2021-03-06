#!/bin/env python3
#
# Authors: Alexander Jung <alexander.jung@neclab.eu>
#

import os
import csv
import sys
import fire
import numpy as np
import collections
from time import gmtime
from time import strftime
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import matplotlib.patches as mpatches
from common import sizeof_fmt, common_style, mk_groups, KBYTES, SMALL_SIZE, MEDIUM_SIZE, LARGE_SIZE

import pprint
pp = pprint.PrettyPrinter(indent=4)

def plot(data=None, output=None):
  WORKDIR = os.getcwd()
  RESULTSDIR = data
  RESULTEXT = '.csv'
  GROUP_BAR_WIDTH = .8
  READ = 'read'
  WRITE = 'write'
  HATCH = r'\\\\'

  files = []
  labels = []
  tsc_nr = {}
  nanos = {}
  tsc_max = 0. # maximum observed tsc
  total_bars = 2
  total_vfs = 0


    # 'sandybrown',
    # # 'darkorange',
    # # 'gold',
    # # 'darkkhaki',
    # # 'yellowgreen',
    # # 'seagreen',
    # # 'turquoise',
    # 'teal',
    # 'deepskyblue',
    # 'lightskyblue',
    # # 'royalblue',
    # # 'mediumpurple',
    # 'orchid',

  project_labels = {
    'unikraft': 'Unikraft',
    'linux': 'Linux'
  }

  vfs_labels = {
    'nomitig-vfs': 'VFS\n(No mitig.)',
    'vfs': 'VFS',
    'shfs': 'SHFS',
    'tlsf-vfs': 'VFS\n(TLSF)',
    'tlsf-shfs': 'SHFS\n(TLSF)'
  }

  vfs_colors = {
    'nomitig-vfs': '#E06E36',
    'vfs': '#3DB2D7',
    'shfs': '#D5316A',
    'tlsf-shfs': '#48AD89',
    'tlsf-vfs': '#C1C1BB',
  }

  file_no_file_labels = {
    'no-file': 'NO FILE',
    'file-exists': 'FILE EXISTS'
  }

  # columns:
  #   experiment
  #   TSC cycles
  #   nanosec

  for f in os.listdir(RESULTSDIR):
    if f.endswith(RESULTEXT):
      index = f.replace(RESULTEXT,'')
      result = index.split('_')

      project = result[0]
      vfs = result[1]

      with open(os.path.join(RESULTSDIR, f), 'r') as csvfile:
        csvdata = csv.reader(csvfile, delimiter="\t")
        
        # next(csvdata) # skip header

        for row in csvdata:
          if project not in tsc_nr:
            tsc_nr[project] = {}
            nanos[project] = {}
          
          if vfs not in tsc_nr[project]:
            tsc_nr[project][vfs] = {}
            nanos[project][vfs] = {}
            total_vfs += 1
            
          tsc_nr[project][vfs][row[0]] = float(row[1])
          nanos[project][vfs][row[0]] = float(row[2])

          if float(row[1]) > tsc_max:
            tsc_max = float(tsc_nr[project][vfs][row[0]])


  pp.pprint(tsc_nr)


  # General style
  common_style(plt)

  # latency_max += 400 # margin above biggest bar

  # Setup matplotlib axis
  fig = plt.figure(figsize=(8, 5))
  renderer = fig.canvas.get_renderer()

  # image size axis
  ax = fig.add_subplot(1,1,1)


  # read latency 
  tsc_max = round(tsc_max) + 2000
  ax.set_ylabel("Average \# TSC")
  # ax.set_xlabel("...")
  ax.grid(which='major', axis='y', linestyle=':', alpha=0.5, zorder=0)
  ax_yticks = np.arange(0, tsc_max, step=1000)
  ax.set_yticks(ax_yticks, minor=False)
  ax_labels = [str(tick) for tick in ax_yticks]
  ax_labels[-1] = '' # remove last label
  ax.set_yticklabels(ax_labels)
  ax.set_ylim(0, ax_yticks[-1])

  # Plot coordinates
  scale = 1. / total_vfs
  xlabels = []


  # Adjust margining
  # fig.subplots_adjust(bottom=.15) #, top=1)

  i = 0
  line_offset = 0
  for project in ['unikraft', 'linux']:
    projects = tsc_nr[project]

    # Write unikernel project on top as "header"
    lxpos = (i + .5 * len(tsc_nr[project].keys())) * scale
    ax.text(lxpos, 1.04, r'\textbf{%s}' % project_labels[project], ha='center',
      transform=ax.transAxes,
      fontweight='bold'
    )

    # Plot a line beteween each project (unikraft, linux, ...)
    if i > 0:
      line = plt.Line2D([i * scale, i * scale], [-.02, 1],
          transform=ax.transAxes, color='black',
          linewidth=1)
      line.set_clip_on(False)
      ax.add_line(line)

    # Plot each project
    for vfs_label in sorted(projects.keys()):
      vfs_tsc = tsc_nr[project][vfs_label]
      vfs_nanos = nanos[project][vfs_label]
      xlabels.append(vfs_labels[vfs_label])

      bar_width = GROUP_BAR_WIDTH / total_bars
      bar_offset = (bar_width / 2) - (GROUP_BAR_WIDTH / 2)

      for file_no_file_label in sorted(vfs_tsc.keys()):
        bar = ax.bar([i + 1 + bar_offset], vfs_tsc[file_no_file_label],
          label=file_no_file_labels[file_no_file_label],
          align='center',
          zorder=4,
          width=bar_width,
          color=vfs_colors[vfs_label],
          hatch=HATCH if file_no_file_label == 'no-file' else None,
          linewidth=.5
        )

        ax.text(i + .97 + bar_offset, vfs_tsc[file_no_file_label] + 100,
          str(round(vfs_tsc[file_no_file_label])) + '\n' +
          '(' + str(round(vfs_nanos[file_no_file_label])) + 'ns)',
          ha='center',
          va='bottom',
          zorder=6,
          fontsize=LARGE_SIZE,
          linespacing=1,
          bbox=dict(pad=-0.6, facecolor='white', linewidth=0),
          rotation='vertical'
        )
        bar_offset += bar_width

      i += 1

  # set up x-axis labels
  xticks = range(1, len(xlabels) + 1)
  ax.set_xticks(xticks)
  # ax.set_xticklabels(xlabels, fontsize=LARGE_SIZE, rotation=45, ha='right', rotation_mode='anchor')
  ax.set_xticklabels(xlabels, fontsize=LARGE_SIZE, fontweight='bold')
  ax.set_xlim(.5, len(xlabels) + .5)
  ax.yaxis.grid(True, zorder=0, linestyle=':')
  ax.tick_params(axis='both', which='both', length=0)

  plt.setp(ax.lines, linewidth=.5)

  # read_handles, read_labels = ax.get_legend_handles_labels()

  file_exists = mpatches.Patch(facecolor='white', label=file_no_file_labels['file-exists'])
  no_file = mpatches.Patch(facecolor='white', label=file_no_file_labels['no-file'], hatch=HATCH)
  leg = plt.legend(handles=[file_exists, no_file], loc='upper left', ncol=2)
  leg.get_frame().set_linewidth(0.0)

  # Save to file
  fig.tight_layout()
  fig.savefig(output, bbox_extra_artists=(ax,), bbox_inches='tight')

if __name__ == '__main__':
  fire.Fire(plot)
