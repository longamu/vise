#!/bin/bash


echo "Starting VISE container ..."

sudo docker run --env USER=$USER --rm -p 9971:9971 -p 9973:9973 -v ~/:/home/$USER -d vise:1.0.0-beta > tmp_container_id.txt

# open web browser
if [ "$(uname)" == "Darwin" ]; then
  # Mac OS X platform
  open http://localhost:9971
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
  # GNU/Linux platform
  sensible-browser http://localhost:9971
else
  echo "This platform is not supported by VISE"
fi

