#!/bin/bash

# syntax:
#   ./create_dataset.sh /data/datasets/vise/data/image/ox5k/oxc1/ /data/datasets/vise/training_time_calibration/data/

FILE_COUNT_LIST=( 50 100 300 700 1100 1700 2600 3700 4900 )
#FILE_COUNT_LIST=( 2 5 )
SOURCE_DIR=$1
DEST_BASEDIR=$2

for file_count in "${FILE_COUNT_LIST[@]}"
do
  echo "Exporting "$file_count" files ..."
  DEST_DIR=$DEST_BASEDIR"/im_set_"$file_count"/"
  mkdir $DEST_DIR

  ls $SOURCE_DIR |sort -R |tail -$file_count |while read file; do
    cp $SOURCE_DIR$file $DEST_DIR$file
    # Something involving $file, or you can leave
    # off the while to just get the filenames
  done

done
