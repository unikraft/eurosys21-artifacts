In order to run this experiment you have to run `run.sh`. 
This program will return a .dot and a .pdf output in the output folder.
For cleaning we use the scrip `clean.sh`

Note: We can use `dot fis.dot -Tpdf -o fis.pdf` to generate a pdf from the.dor

Note2: We use the following two aesthetic options in the .dot file from the paper:
edge [ fontname=Helvetica, fontcolor=blue, fontsize=35 ];
node [ fontname=Helvetica, fontcolor=black, fontsize=80,shape=box];	

Dependencies:
* cscope
* git
* graphviz

Scripts description:
* `component_symbols.sh` -> for each symbol in a defined module(e.g. net) we find all the symbols using cscope. For each symbol we find the source module. 

