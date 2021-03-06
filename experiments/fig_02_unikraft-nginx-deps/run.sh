#!/bin/bash
# Modify tmpdir, on my end it would fail if I was using tmpfs
mkdir tmp
export TMPDIR=$PWD/tmp

cscope -b -R

./component_symbols.sh

python3 plot.py
