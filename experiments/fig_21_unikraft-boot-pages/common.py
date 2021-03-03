#!/bin/env python3
#
# Authors: Alexander Jung <alexander.jung@neclab.eu>
#

import matplotlib as mpl
import matplotlib.pyplot as plt

SMALL_SIZE = 12
MEDIUM_SIZE = 14
LARGE_SIZE = 18
BIGGER_SIZE = 24
KBYTES = 1024.0

PATTERNS = ('-', '+', 'x', '\\', '.')

def common_style(plt):
  plt.style.use('classic')
  plt.tight_layout()

  plt.rcParams['text.usetex'] = False
  plt.rc('pdf', fonttype=42)

  # plt.rcParams["font.family"] = "Arial"

  # plt.rc('font', **{
  #   'family': 'sans-serif',
  #   'sans-serif': ['DejaVu Sans'] # ['Computer Modern']
  # })

  plt.rc('font',**{
    'family':'sans-serif',
    'sans-serif':['Helvetica']}
  )
  plt.rc('text', usetex=True)


  # plt.rcParams['font.sans-serif'] = "Comic Sans MS"
  plt.rcParams['font.family'] = "sans-serif"

  plt.rc('font', size=MEDIUM_SIZE)         # controls default text sizes
  plt.rc('axes', titlesize=MEDIUM_SIZE)    # fontsize of the axes title
  plt.rc('axes', labelsize=LARGE_SIZE)     # fontsize of the x and y labels
  plt.rc('xtick', labelsize=LARGE_SIZE)   # fontsize of the tick labels
  plt.rc('ytick', labelsize=MEDIUM_SIZE)   # fontsize of the tick labels
  plt.rc('legend', fontsize=MEDIUM_SIZE)   # legend fontsize
  # plt.rc('figure', titlesize=BIGGER_SIZE, titleweight='bold')  # fontsize of the figure title

def mk_groups(data, selector='mean'):
    try:
        newdata = data.items()
    except:
        return

    thisgroup = []
    groups = []

    for key, value in newdata:
      newgroups = mk_groups(value, 'mean')

      if isinstance(value, dict) and selector in value:
        thisgroup.append((key, value[selector]))
        
      elif newgroups is None:
        thisgroup.append((key, value))

      else:
        thisgroup.append((key, len(newgroups[-1])))
        
        if groups:
          groups = [g + n for n, g in zip(newgroups, groups)]
        else:
          groups = newgroups
      
    return [thisgroup] + groups

def add_line(ax, xpos, ypos, height=.1):
    line = plt.Line2D([xpos, xpos], [ypos, ypos + height],
                      transform=ax.transAxes, color='black',
                      linewidth=1)
    line.set_clip_on(False)
    ax.add_line(line)

def sizeof_fmt(num, suffix='B', last_zero=False):
  for unit in ['','K','M','G']:
    if abs(num) < KBYTES:
      if num == 0 or num.is_integer():
        return "%d%s%s" % (int(num), unit, suffix)
      else:
        return "%3.1f%s%s" % (num, unit, suffix)
    num /= KBYTES
  return "%.1f%s%s" % (num, 'Yi', suffix)
