#!/bin/bash

echo "Loading VISE docker image ..."
echo "(Note: this may take some time)"

sudo docker load --input vise_docker-1.0.0.tar --quiet
