#!/bin/bash

# Script to train VISE system on a custom set of images. This allows the VISE
# system to be used to search this custom set of images.
#
#
#
# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# 20 March 2017
#

VISE_ROOTDIR="/home/tlm/dev/vise/"

printf "Script to train the VGG Image Search Engine (VISE)"
printf "\nFor more details, see www.robots.ox.ac.uk/~vgg/software/vise"
printf "\n\n"
#
# Ask the user to point to the location of all images
#
read -e -p "Enter the location of images (default: ~/data/datasets/vise/data/image/sample_training/images/) : " IMAGE_DIR

if [[ -z "$IMAGE_DIR" ]]; then
  IMAGE_DIR="~/data/datasets/vise/data/image/sample_training/images/"
fi
printf "\nIMAGE_DIR= "$IMAGE_DIR

if [[ ! -d "`eval echo ${IMAGE_DIR//>}`" ]]; then
  printf "\n\tImages directory does not exist! "$IMAGE_DIR"\n"
  exit
fi

IMAGE_DIR=`eval echo ${IMAGE_DIR//>}`
TRAIN_DATA_DIR=$IMAGE_DIR"../vise_traindata/"
TRAIN_IMG_LIST=$TRAIN_DATA_DIR"img_list.txt"

# expand ~ to home directory before checking dir. for existence
if [[ ! -d "`eval echo ${TRAIN_DATA_DIR//>}`" ]]; then
  mkdir -p $TRAIN_DATA_DIR
fi
printf "\nTRAIN_DATA_DIR= "$TRAIN_DATA_DIR

printf "\nimlist= "$TRAIN_IMG_LIST
(cd $IMAGE_DIR; find . -type f > $TRAIN_IMG_LIST)

#
# Build image list
#

#
# train desc
#
printf "\ntrainDescs : \n"
TRAIN_DESC=$TRAIN_DATA_DIR"train_descs.e3bin"

if [[ ! -f $TRAIN_DESC ]]; then
  (cd $VISE_ROOTDIR"build"; 
  date ;
  ./v2/indexing/compute_index_v2 trainDescs sample_training ;
  date )
else 
  echo "*** Skipped trainDescs"
fi

#
# Computer clusters
#
printf "\ncompute_clusters : \n"
CLST=$TRAIN_DATA_DIR"clst.e3bin"

if [[ ! -f $CLST ]]; then
  (cd $VISE_ROOTDIR"build" ; 
  date ;
  python ../src/v2/indexing/compute_clusters.py sample_training ;
  date
  )
else
  echo "*** Skipped compute_clusters"
fi

#
# train assign
#
printf "\ntrainAssign : \n"
TRAIN_DESC=$TRAIN_DATA_DIR"train_*_ assign.e3bin"

if [[ ! -f $TRAIN_DESC ]]; then
  (cd $VISE_ROOTDIR"build"; 
  date ;
  ./v2/indexing/compute_index_v2 trainAssign sample_training ;
  date )
else 
  echo "*** Skipped trainAssign"
fi


#
# train hamm
#
printf "\ntrainHamm : \n"
TRAIN_DESC=$TRAIN_DATA_DIR"train_hamm.e3bin"

if [[ ! -f $TRAIN_DESC ]]; then
  (cd $VISE_ROOTDIR"build"; 
  date ;
  ./v2/indexing/compute_index_v2 trainHamm sample_training ;
  date )
else 
  echo "*** Skipped trainHamm"
fi

#
# perform indexing
#
printf "\nindex : \n"
TRAIN_DESC=$TRAIN_DATA_DIR"train_index.e3bin"

if [[ ! -f $TRAIN_DESC ]]; then
  (cd $VISE_ROOTDIR"build"; 
  date ;
  ./v2/indexing/compute_index_v2 index sample_training ;
  date )
else 
  echo "*** Skipped index"
fi
printf "\nDone\n"
