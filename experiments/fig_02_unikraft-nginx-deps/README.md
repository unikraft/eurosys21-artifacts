# Unikraft NGINX Dependency Graph

In order to run this experiment you have to run `run.sh`.  This will download
Unikraft from github and checkout to commit `775dc01`.  This program will return
a `.dot` and a `.pdf` output in the output folder.

  **N.B.**: We can use:the following to generete a pdf from the.dor:
  ```bash
  dot fis.dot -Tpdf -o fis.pdf
  ```

Dependencies

 * cscope
 * git
 * graphviz

Scripts description:

 * `component_symbols.sh`: for each symbol in a defined module(e.g. net) we find
   all the symbols using cscope. For each symbol we find the source module. 
