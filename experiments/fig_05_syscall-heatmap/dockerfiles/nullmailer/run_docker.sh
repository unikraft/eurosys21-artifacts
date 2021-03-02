#!/bin/bash
NAME=nullmailer
echo "Building the docker image for $NAME"
docker build -t "$NAME" .
echo "Running the docker image for $NAME. This can take some time..."
docker run --name "$NAME" --rm "$NAME" > "$NAME.json"
echo "Output is saved into $NAME.json"
