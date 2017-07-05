#!/bin/bash
echo ""
echo "--------------------------------------------------"
echo "Shutting down VISE 1.0.1 docker container"
echo "(This may take some time)"
echo "--------------------------------------------------"
echo ""

sudo docker stop `sudo docker ps --filter ancestor=vise:1.0.1 -a -q`
sudo docker rm `sudo docker ps --filter ancestor=vise:1.0.1 -a -q`
