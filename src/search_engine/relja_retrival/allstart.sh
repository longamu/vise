#!/bin/bash

if [ $# -ne 3 ]; then
  echo "Usage: `basename $0` webport demoname apiport"
  exit $E_BADARGS
fi

webport=$1
demoname=$2
apiport=$3

echo "Starting demo: "$demoname
echo "webport:       "$webport
echo "apiport:       "$apiport

rm start.log > /dev/null

screen -S api_$demoname -d -m ./api_run.sh $apiport $demoname

screen -S web_$demoname -d -m ./web_run.sh $webport $demoname $apiport
