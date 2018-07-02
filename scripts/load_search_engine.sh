#!/bin/sh

if [ "$#" -ne 1 || "$#" -ne 3 ]; then
  echo "${0} SEARCH_ENGINE_NAME"
  exit
fi

NAME=$1
VERSION=1
BACKEND_PORT=10001
FRONTEND_PORT=10002
if [ -z "$2" ]; then
  BACKEND_PORT=${2}
fi
if [ -z "$3" ]; then
  FRONTEND_PORT=${3}
fi

REPO_DIR="/home/tlm/mydata/vise/repo"

if screen -list | grep -q "${NAME}_backend"; then
  echo "Closing existing ${NAME} backend ..."
  screen -X -S "${NAME}_backend" quit 
fi

echo "Loading backend for "$NAME
echo screen -dmS "${NAME}_backend" sh -c "LD_LIBRARY_PATH=\"/home/tlm/deps/vise/lib/lib\" /home/tlm/dev/vise_stable/vise/bin/api_v2 10001 ${NAME} \"${REPO_DIR}/${NAME}/${VERSION}/vise_data/config.txt\""

if screen -list | grep -q "${NAME}_frontend"; then
  echo "Closing existing ${NAME} frontend ..."
  screen -X -S "${NAME}_frontend" quit 
fi

echo "Loading frontend for "$NAME
echo screen -dmS "${NAME}_frontend" sh -c "python /home/tlm/dev/vise/src/search_engine/relja_retrival/src/ui/web/webserver.py 10002 ${NAME} 10001 0 \"${REPO_DIR}/${NAME}/${VERSION}/vise_data/config.txt\""


echo "done"
screen -ls
