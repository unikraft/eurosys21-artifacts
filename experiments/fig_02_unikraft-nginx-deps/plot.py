#!/bin/env python3
import os
import fire
from graphviz import Digraph

def plot(data=None, output=None):
    components=["lib-newlib" ,"posix-libdl" ,"posix-process" ,"posix-user" ,"ramfs" ,"ukalloc" ,"ukallocbbuddy" ,"uklock" ,"ukmpi" ,"uknetdev" ,"uksched" ,"ukschedcoop" , "ukswrand" ,"uktime" ,"uktimeconv" ,"vfscore", "lib-lwip"]

    dot = Digraph(comment='Nginx graph plot')
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
    **dict.fromkeys(["lib-newlib", "posix-process", "posix-user", "posix-libdl"], "posix-layer"),
    **dict.fromkeys(["vfscore", "ramfs"], "fs"),
    **dict.fromkeys(["uktime", "uktimeconv"], "time"),
    **dict.fromkeys(["uklock"], "locking"),
    **dict.fromkeys(["uksched", "ukschedcoop"], "sched"),
    **dict.fromkeys(["ukswrand"], "security"),
    **dict.fromkeys(["ukalloc", "ukallocbbuddy"], "mm"),
    **dict.fromkeys(["uknetdev", "lib-lwip"], "net"),
    **dict.fromkeys(["ukmpi"], "ipc"),
    **dict.fromkeys(["uksglist"], "ukds"),
    **dict.fromkeys(["ukboot"], "ukboot1"),
    **dict.fromkeys(["ukargparse"], "ukargparse1"),
    **dict.fromkeys(["ukbus"], "ukbus1"),
    }

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
        continue
        t = "cluster"+i
        with dot.subgraph(name=t) as c:
            c.attr(label=i)
            c.attr(pack="8")
            c.attr(style="filled")
            c.attr(color="lightgrey")
            c.attr(overlap='false')
            c.attr(style='rounded, filled')
            c.attr(fontsize='10')

            #for j, value2 in myDict.items():
                #if value2 == i:
                    #c.node(j, shape="box", style="filled, rounded",color="white",fontsize="12")


    for i, value in G.items():
        for j, value2 in value.items():
            if i != j:
                dot.edge(i,j, label=str(value2))

    # indirect calls are missing, we add them manually
    dot.edge("posix-layer", "mm", "6")
    dot.edge("sched", "mm", "2")
    dot.attr("node", fontcolor="white", fontsize="80", shape="box", style="filled", color="black")
    dot.edge("nginx", "posix-layer")
    dot.edge("nginx", "net")
    dot.render(output)

if __name__ == '__main__':
    fire.Fire(plot)
