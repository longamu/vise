#!/bin/bash

VGG_DIR="$HOME/vgg"
VGG_MYDATA_DIR="$HOME/vgg/mydata/images"
VGG_VISE_DIR="$HOME/vgg/vise"

if [ ! -d "${VGG_DIR}" ]; then
  echo "Creating directory ${VGG_DIR} ..."
  mkdir -p "${VGG_DIR}"
fi

if [ ! -d "${VGG_MYDATA_DIR}" ]; then
  echo "Creating directory ${VGG_MYDATA_DIR} ..."
  mkdir -p "${VGG_MYDATA_DIR}"
fi

if [ ! -d "${VGG_VISE_DIR}" ]; then
  echo "Creating directory ${VGG_VISE_DIR} ..."
  mkdir -p "${VGG_VISE_DIR}"
fi

# create symbolic link to vgg in desktop
if [ ! -h "${HOME}/Desktop/vgg" ]; then 
  echo "Creating symblic link to ${VGG_DIR} in Desktop ..."
  ln -s "${VGG_DIR}" "${HOME}/Desktop/vgg"
fi

if [[ $(sudo docker images -q vise:1.0.1) ]]; then
    echo "--------------------------------------------------"
    echo "VISE Docker image is already loaded."
    echo "Recall that you have to run the load script only once"
    echo "--------------------------------------------------"
else 
    echo ""
    echo "--------------------------------------------------"
    echo "Loading VISE 1.0.1 docker image ..."
    echo "(Note: this may take some time)"
    echo "--------------------------------------------------"
    echo ""
    SCRIPT_DIR=$(dirname "$0")

    sudo docker load --input "${SCRIPT_DIR}/vise-1.0.1.tar"

fi
