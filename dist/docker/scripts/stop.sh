#!/bin/bash


echo "Stopping VISE container ..."

# @todo: stop only the VISE container
sudo docker stop `sudo docker ps -a -q`
