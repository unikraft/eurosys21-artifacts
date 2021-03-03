from graphviz import Digraph

components=["nolibc", "ukalloc", "ukallocbbuddy", "ukboot", "ukargparse"]

dot = Digraph(comment='Hello graph plot', engine="fdp")
dot.attr(ratio="compress")
dot.attr(ranksep="0")
dot.attr(concentrate="true")
dot.attr(size="50,20")
dot.attr(rankdir="LR")
#for i in components:
#    dot.node(i)

adj_list = {}


for i in components:
    adj_list[i] = {}
    for line in  open(i + ".deps", "r"):
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
        c.attr(fontsize='10')

        for j, value2 in myDict.items():
            if value2 == i:
                c.node(j, shape="box", style="filled, rounded",color="white",fontsize="12")


for i, value in G.items():
    for j, value2 in value.items():
        if i != j:
            dot.edge("cluster"+i,"cluster"+j, label=str(value2))

dot.render('output/unikraft_hello.gv', view=True)
