#!/bin/bash
python3 -m pip install xlrd==1.2.0
python3 -m pip install numpy==1.16.5
python3 -m pip install seaborn

if [ $(id -u) -eq 0 ]; then
    echo "Already root for docker commands"
else
    sudo usermod -aG docker $USER
fi

mkdir results
cd dockerfiles/base
docker build -t debian_env .
cd ../../dockerfiles
for d in *; do
     if [[ -d $d ]] && [ "${d}" != "base" ]; then
        echo "Enter to $d repository"
        cd $d
        echo "Building the docker image for $d"
        docker build -t "$d" .
        cd ..
        echo "---------------------------------------------"
    fi
done