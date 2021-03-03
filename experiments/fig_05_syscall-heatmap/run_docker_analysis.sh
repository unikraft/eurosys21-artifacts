#!/bin/bash
mkdir aggregated_dockerfile
cd dockerfiles
cd base
docker build -t debian_env .
cd ../..
cd dockerfiles
for d in *; do
     if [[ -d $d ]] && [ "${d}" != "base" ]; then
        echo "Enter to $d repository"
        cd $d
        echo "Building the docker image for $d"
        docker build -t "$d" .
        echo "Running the docker image for $d. This can take some time..."
        docker run --name "$d" --rm "$d" > "$d.json"
        echo "Output is saved into $d.json"
        cp "$d.json" ../../aggregated_dockerfile/"$d.json"
        cd ..
        echo "---------------------------------------------"
    fi
done