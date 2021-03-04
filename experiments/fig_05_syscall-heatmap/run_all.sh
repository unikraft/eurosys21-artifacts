#!/bin/bash
cd dockerfiles
for d in *; do
     if [[ -d $d ]] && [ "${d}" != "base" ]; then
        echo "Enter to $d repository"
        cd $d
        echo "Running the docker image for $d. This can take some time..."
        docker run --pull=never --name "$d" --rm "$d" > "$d.json"
        echo "Output is saved into $d.json"
        mv "$d.json" ../../aggregated_dockerfile/"$d.json"
        cd ..
        echo "---------------------------------------------"
    fi
done