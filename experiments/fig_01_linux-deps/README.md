# Linux Dependency Graph

<img align="right" src="../../plots/fig_01_linux-deps" width="300" />
This experiment demonstrates the complexity of the Linux kernel.  To quote the
paper, "to better quantify this API complexity, we analyzed dependencies between
the main components of the Linux kernel.  As a rough approximation, we used the
subdirectories in the kernel source tree to identify (broad) components.  We
used `cscope` to extract all function calls from the sources of all kernel
components, and then for each call checked to see if the function is defined in
the same component or a different one; in the latter case, we recorded a
dependency.  We plot the dependency graph in [this figure]: the annotations on
the edges show the number of dependencies between nodes.  This dense graph makes
it obvious that removing or replacing any single component in the Linux kernel
requires understanding and fixing all the dependencies of other components, a
daunting task."

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

