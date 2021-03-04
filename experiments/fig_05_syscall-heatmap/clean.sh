#!/bin/bash
cd dockerfiles
for d in *; do
    if [[ -d $d ]] && [ "${d}" != "base" ]; then
        echo "[$d]"
        docker rmi $(docker images "$d" -a -q)
    fi
done
docker rmi $(docker images 'debian_env' -a -q)