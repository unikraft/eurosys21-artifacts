#!/bin/env python3
import os
import fire
from graphviz import Digraph

def plot(data=None, output=None):
    components=["fs", "ipc", "mm", "net", "security", "block","sched","time","irq","locking"]

    dot = Digraph(comment='Linux graph plot')
    dot.attr(ratio="compress")
    dot.attr(ranksep="0")
    dot.attr(concentrate="true")
    dot.attr(size="50,20")
    dot.attr(rankdir="LR")

    dot.attr("node", fontname="Helvetica", fontcolor="black", fontsize="80",shape="box")
    dot.attr("edge", fontname="Helvetica", fontcolor="blue", fontsize="35")

    adj_list = {}

    for i in components:
        adj_list[i] = {}
        for line in  open(os.path.join(data, i) + ".deps", "r"):
            if "linux" in line:

                comp = ""

                for w in components:
                    if w in line:
                        comp = w
                        break
                
                if comp == "":
                    continue

                j = comp

                if j == i or j == "ukdebug":
                    continue

                if j not in adj_list[i]:
                    adj_list[i][j] = 0


                adj_list[i][j] = adj_list[i][j] + 1


    for i in components:
        for j in adj_list[i]:
            if j in components:
                dot.edge(i,j, label=str(adj_list[i][j]))

    dot.render(output)

if __name__ == '__main__':
    fire.Fire(plot)
