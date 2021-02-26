In order to run this experiment you have to run run.sh. This will download linux from github and checkout to commit c85fb28b6f999db9928b841f63f1beeb3074eeca.
This program will return a .dot description of the dependency graph. We can use `dot fis.dot -Tpdf -o fis.pdf` to generate a pdf.
For cleaning we use the scrip `clean.sh`

Dependencies:
* cscope
* git

Scripts description:
* `create_cscope_db.sh` -> creates the cscope database of used files. We exclude drivers, arch etc.
* `component_symbols.sh` -> for each symbol in a defined module(e.g. net) we find all the symbols using cscope. For each symbol we find the source module. 

