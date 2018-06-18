#!/bin/bash

if [ $# -ne 2 ]; then
  echo "Usage: `basename $0` apiport demoname"
  exit $E_BADARGS
fi

apiport=$1
demoname=$2

# This script is here to be run as screen ./scriptname
# Do not run screen inside of this - problems with loading .bash_profile (setting LD_LIBRARY_PATH)

source ~/.bash_profile

cd build

while true; do
    echo -n "starting api: "
    date
    echo -n "starting api: " >> start.log
    date >> start.log
    v2/api/api_v2 $apiport $demoname
    sleep 1
done
