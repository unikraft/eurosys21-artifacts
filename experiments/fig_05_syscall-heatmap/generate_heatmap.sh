#!/bin/bash
python3 -m pip install xlrd==1.2.0
python3 -m pip install seaborn
python3 heatmap.py --save-heatmap --folder-to-aggregate aggregated_dockerfile