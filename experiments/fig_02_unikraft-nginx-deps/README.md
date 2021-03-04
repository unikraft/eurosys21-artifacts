In order to run this experiment you have to run run.sh. This will download unikraft from github and checkout to commit 775dc0123e8523008463fe6b28d45b87a836ae89.
This program will return a .dot and a .pdf output in the output folder.
For cleaning we use the scrip `clean.sh`

Note: We can use `dot fis.dot -Tpdf -o fis.pdf` to generate a pdf from the.dor

Dependencies:
* cscope
* git
* graphviz

Scripts description:
* `component_symbols.sh` -> for each symbol in a defined module(e.g. net) we find all the symbols using cscope. For each symbol we find the source module. 

