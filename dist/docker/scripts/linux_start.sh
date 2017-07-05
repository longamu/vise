#!/bin/bash

if [[ $(sudo docker ps --filter ancestor=vise:1.0.1 -a -q) ]]; then
    echo "--------------------------------------------------"
    echo "Stopping past sessions of VISE (may take some time)..."
    echo "--------------------------------------------------"
    sudo docker stop `sudo docker ps --filter ancestor=vise:1.0.1 -a -q`
    sudo docker rm `sudo docker ps --filter ancestor=vise:1.0.1 -a -q`
fi

echo ""
echo "--------------------------------------------------"
echo "Starting VISE 1.0.1 ..."
echo "--------------------------------------------------"

# open web browser
if [ "$(uname)" == "Darwin" ]; then
  # Mac OS X platform
  sudo docker run --env USER=$USER --env HOME=$HOME -p 9971:9971 -p 9973:9973 -v ~/:/Users/$USER -d vise:1.0.1
  open http://localhost:9971 &
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
  # GNU/Linux platform
  sudo docker run --env USER=$USER --env HOME=$HOME -p 9971:9971 -p 9973:9973 -v ~/:/home/$USER -d vise:1.0.1
  sensible-browser http://localhost:9971 &
else
  echo "This platform is not supported by VISE"
fi

