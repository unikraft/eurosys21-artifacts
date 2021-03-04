#!/bin/bash
DIR="aggregated_dockerfile"
if [ -d "$DIR" ]; then
    python3 heatmap.py --save-heatmap --folder-to-aggregate $DIR
else
    echo "$DIR does not exist. Perform the unikraft syscall experiment first"
fi
