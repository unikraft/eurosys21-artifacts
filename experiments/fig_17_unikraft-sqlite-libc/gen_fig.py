#!/usr/bin/env python3

import glob
import matplotlib.pyplot as plt
import numpy as np
import os
import sys

labels = {
        'app-sqlite-linux-native.dat': 'Linux\n(native)',
        'app-sqlite-newlib-native.dat': 'newlib\n(native)',
        'app-sqlite-musl-native.dat': 'musl\n(native)',
        'app-sqlite-musl-compat.dat': 'musl\n(external)',
        }

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: {} <results folder> <figure.pdf>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    os.chdir(sys.argv[1])

    stats = {}
    max_time = 0
    for fn in glob.glob("*.dat"):
        data = np.loadtxt(fn)
        avg = np.average(data)
        std = np.std(data)
        stats[fn] = {
                'min': avg - std,
                'avg': avg,
                'max': avg + std,
                }
        if stats[fn]['max'] > max_time:
            max_time = stats[fn]['max']

    # TODO import general style
    LARGE_SIZE = 18

    max_time *= 1.2 # Margin above biggest bar

    fig = plt.figure(figsize=(8, 4))
    ax = fig.add_subplot(1, 1, 1)
    ax.set_ylabel("Time (seconds)")
    ax.grid(which='major', axis='y', linestyle=':', alpha=0.5, zorder=0)
    yticks = np.arange(0, max_time, step=1)
    ax.set_yticks(yticks, minor=False)
    ax.set_yticklabels(["%3.0f" % ytick for ytick in yticks])
    ax.set_ylim(0, max_time)

    xlabels = []
    i = 0
    for experiment in labels.keys():
        xlabels.append(labels[experiment])
        time = stats[experiment]

        yerr = time['max'] - time['min']
        print(experiment, time['avg'], '+/-', yerr/2)

        # Plot each application
        bar = ax.bar([i + 1], time['avg'],
                label=experiment,
                align='center',
                zorder=4,
                yerr=time['max']-time['min'],
                error_kw=dict(lw=2, capsize=10, capthick=1),
                width=0.4,
                color='#5697C4',
                linewidth=0.5
                )
        ax.text(i + 1, time['avg'] + yerr + .1, "%3.3f" % time['avg'],
                ha='center',
                va='bottom',
                zorder=6,
                fontsize=LARGE_SIZE,
                linespacing=0,
                bbox=dict(pad=-.6, facecolor='white', linewidth=0),
                rotation='horizontal'
                )
        i += 1

    xticks = range(1, len(xlabels) + 1)
    ax.set_xticks(xticks)
    ax.set_xticklabels(xlabels, fontsize=LARGE_SIZE, fontweight='bold')
    ax.set_xlim(.5, len(xlabels) + .5)
    ax.yaxis.grid(True, zorder=0, linestyle=':')
    ax.tick_params(axis='both', which='both', length=0)

    plt.setp(ax.lines, linewidth=.5)

    fig.tight_layout()
    fig.savefig(sys.argv[2])
