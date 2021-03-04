#!/bin/bash
SRC="${1:-./src}"
set -e
set -x

cd "$SRC"
make clean syscallbench
