#!/bin/bash
echo ""
echo "--------------------------------------------------"
echo "Shutting down VISE 1.0.1 docker container"
echo "(This may take some time)"
echo "--------------------------------------------------"
echo ""

if [ "$(uname)" == "Darwin" ]; then
  # Mac OS X platform
  docker stop `docker ps --filter ancestor=vise:1.0.1 -a -q`
  docker rm `docker ps --filter ancestor=vise:1.0.1 -a -q`
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
  # GNU/Linux platform
  sudo docker stop `sudo docker ps --filter ancestor=vise:1.0.1 -a -q`
  sudo docker rm `sudo docker ps --filter ancestor=vise:1.0.1 -a -q`
else
  echo "This platform is not supported by VISE"
fi

