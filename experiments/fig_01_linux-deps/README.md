# Linux Dependency Graph

In order to run this experiment you have to run run.sh.  This will download
Linux from github and checkout to commit `c85fb28`.  This program will return a
`.dot` and a .pdf output in the output folder.  For cleaning we use the script
`clean.sh`

  **N.B.**: We can use:the following to generete a pdf from the.dor:
  ```bash
  dot fis.dot -Tpdf -o fis.pdf
  ```

  **N.B.B.**: We use the following two aesthetic options in the `.dot` file from
  the paper:
  ```
  edge [ fontname=Helvetica, fontcolor=blue, fontsize=35 ];
  node [ fontname=Helvetica, fontcolor=black, fontsize=80,shape=box];	
  ```

Dependencies:
* cscope
* git
* graphviz

Scripts description:
* `create_cscope_db.sh` -> creates the cscope database of used files. We exclude drivers, arch etc.
* `component_symbols.sh` -> for each symbol in a defined module(e.g. net) we find all the symbols using cscope. For each symbol we find the source module. 

