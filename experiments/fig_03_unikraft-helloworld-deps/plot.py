#!/bin/env python3
import os
import fire
from graphviz import Digraph

def plot(data=None, output=None):
    components=["nolibc", "ukalloc", "ukallocbbuddy", "ukboot", "ukargparse"]

    dot = Digraph(comment='Hello graph plot', engine="fdp")
    dot.attr(ratio="compress")
    dot.attr(ranksep="0")
    dot.attr(concentrate="true")
    dot.attr(size="50,20")
    dot.attr(rankdir="LR")
    dot.attr("node", fontname="Helvetica", fontcolor="black", fontsize="30",shape="box")
    dot.attr("edge", fontname="Helvetica", fontcolor="blue", fontsize="15")

    #for i in components:
    #    dot.node(i)

    adj_list = {}


    for i in components:
        adj_list[i] = {}
        for line in  open(os.path.join(data, i) + ".deps", "r"):
            if "lib" in line:
                j = line.split("/")[2]
                if j == i or j == "ukdebug":
                    continue

                if j not in components:
                    continue

                if j not in adj_list[i]:
                    adj_list[i][j] = 0

                adj_list[i][j] = adj_list[i][j] + 1


    myDict = {
    **dict.fromkeys(["nolibc"], "posix-layer"),
    **dict.fromkeys(["ukalloc", "ukallocbbuddy"], "mm"),
    **dict.fromkeys(["ukboot"], "ukboot1"),
    **dict.fromkeys(["ukargparse"], "ukargparse1"),
    }
    # indirect calls
    adj_list["ukallocbbuddy"]["ukalloc"] += 2
    G = {}
    for i in components:
        G[myDict[i]] = {}

        for j, value in adj_list[i].items():
            #print(myDict[j])
            if myDict[j] not in G[myDict[i]]:
                G[myDict[i]][myDict[j]] = 0
        
            G[myDict[i]][myDict[j]] += value 
            #dot.edge(i, j, label=str(value))

    for i, value in G.items():
        #print("#####  " + i)
        t = "cluster"+i
        with dot.subgraph(name=t) as c:
            c.attr(label=i)
            c.attr(pack="8")
            c.attr(style="filled")
            c.attr(color="lightgrey")
            c.attr(overlap='false')
            c.attr(style='rounded, filled')
            c.attr(fontsize='20')

            for j, value2 in myDict.items():
                if value2 == i:
                    c.node(j, shape="box", style="filled, rounded",color="white",fontsize="12")

    for i in components:
        for j in components:
            if i != j and j in adj_list[i] and adj_list[i][j] > 0:
                dot.edge(i,j, label=str(adj_list[i][j]))

    dot.edge("nolibc", "ukalloc", "6")
    dot.attr("node", fontcolor="white", fontsize="25", shape="box", style="filled", color="black")
    dot.edge("Hello World", "nolibc", "1")
    dot.render(output)

if __name__ == '__main__':
    fire.Fire(plot)
