#!/bin/bash

# Script to run the VISE demo
#
#
#
# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# 16 March 2017
#

echo "========================================="
echo "     VGG Image Search Engine (VISE)"
echo "========================================="
echo ""
echo "Which demo would your like to run?"
echo ""
echo "[1] 15th Century Book"
echo "[2] Oxford Buildings"
echo "[3] Bodleian Library Ballads"
echo ""
printf "Enter your choice (1 - 3) : "
read demo_id

BACKEND_PORT=35280
FRONTEND_PORT=8080
VISE_DATASET_NAME=""

case "$demo_id" in
    "1")
        VISE_DATASET_NAME="15c_bt"
        ;;
    "2")
        VISE_DATASET_NAME="ox5k"
        ;;
    "3")
        VISE_DATASET_NAME="Ballads"
        ;;
esac

echo ""
echo "Running VISE on dataset "$VISE_DATASET_NAME

cd $VISE_BUILDDIR
DYLD_FALLBACK_LIBRARY_PATH=/Users/tlm/dev/vise/install_scripts/../dep/lib/fastann/lib/:/Users/tlm/dev/vise/install_scripts/../dep/lib/protobuf/lib/:/Users/tlm/dev/vise/install_scripts/../dep/lib/boost/lib/ v2/api/api_v2 $BACKEND_PORT $VISE_DATASET_NAME &
BACKEND_PID=$!

echo "Waiting for backend to start ..."
sleep 3

echo "Starting frontend ..."
python $VISE_ROOTDIR"src/ui/web/webserver.py" $FRONTEND_PORT $VISE_DATASET_NAME $BACKEND_PORT &
FRONTEND_PID=$!

sleep 2
open http://localhost:8080

printf "Press any key to stop everything! "
read any_key
kill $FRONTEND_PID
kill $BACKEND_PID

echo "Bye"
