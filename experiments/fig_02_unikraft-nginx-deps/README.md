# Unikraft NGINX Dependency Graph

<img align="right" src="../../plots/fig_02_unikraft-nginx-deps.svg" width="300" />

In contrast to [`fig_01`](../fig_01) we demonstrate the simplicty of a [Unikraft
NGINX unikernel](https://github.com/unikraft/app-nginx).  To quote the paper,
"the annotations on the edges show the number of dependencies between nodes. ...
We used [`cscope`](http://cscope.sourceforge.net/) to extract all function calls
from the sources of all kernel components, and then for each call checked to see
if the function is defined in the same component or a different one; in the
latter case, we recorded a dependency."

In order to run this experiment you have to run `run.sh`.  This will download
Unikraft from github and checkout to commit `775dc01`.  This program will return
a `.dot` and a `.pdf` output in the output folder.

  **N.B.**: We can use:the following to generete a pdf from the.dor:
  ```bash
  dot fis.dot -Tpdf -o fis.pdf
  ```

Dependencies

 * `cscope`
 * `git`
 * `graphviz`

Scripts description:

 * `component_symbols.sh`: for each symbol in a defined module(e.g. net) we find
   all the symbols using cscope. For each symbol we find the source module. 
