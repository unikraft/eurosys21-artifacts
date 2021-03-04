#!/bin/bash
DIR="aggregated_dockerfile"
if [ -d "$DIR" ]; then
    python3 cruncher.py --plot
else
    echo "$DIR does not exist. Perform the unikraft syscall experiment first"
fi
