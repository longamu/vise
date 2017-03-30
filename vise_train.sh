#!/bin/bash

set -e
set -u

# Script to train VISE system on a custom set of images. This allows the VISE
# system to be used to search this custom set of images.
#
#
#
# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# 20 March 2017
#

VISE_ROOTDIR=$(pwd)

printf "Script to train the VGG Image Search Engine (VISE)"
printf "\nFor more details, see www.robots.ox.ac.uk/~vgg/software/vise"
printf "\n\n"
#
# Ask the user to point to the location of all images
#
read -e -p "Enter the location of images (default: ox5k): " IMAGE_DIR

if [[ $IMAGE_DIR != *[/]* ]]; then
    if [[ -z "$IMAGE_DIR" ]]; then
        #echo "You must enter the location of images"
        #exit
        IMAGE_DIR="~/data/datasets/vise/data/image/ox5k/oxc1"
    else
        IMAGE_DIR="~/data/datasets/vise/data/image/"$IMAGE_DIR
    fi
fi

if [[ ! -d "`eval echo ${IMAGE_DIR//>}`" ]]; then
  printf "\n\tImages directory does not exist! "$IMAGE_DIR"\n"
  exit
fi
printf "\tVISE_ROOTDIR = "${VISE_ROOTDIR}
printf "\n\tImage location = "${IMAGE_DIR}

DATASET_NAME=$(basename ${IMAGE_DIR})
printf "\n\tDATASET = "${DATASET_NAME}

IMAGE_DIR=`eval echo ${IMAGE_DIR//>}`
VISE_DATADIR="${IMAGE_DIR}/../${DATASET_NAME}_visedata/"

IMLIST="${VISE_DATADIR}imlist.txt"

# expand ~ to home directory before checking dir. for existence
if [[ ! -d "${VISE_DATADIR}" ]]; then
  mkdir -p $VISE_DATADIR
fi
printf "\n\tVISE_DATADIR= "$VISE_DATADIR

TMPDIR="${VISE_DATADIR}tmp/"
if [[ ! -d "${TMPDIR}" ]]; then
    mkdir $TMPDIR
fi

#
# Build image list
#
printf "\n\timlist= "$IMLIST
(cd $IMAGE_DIR; find . -type f > $IMLIST)

#
# Build configuration file
#
CONF="${VISE_DATADIR}conf.txt"
echo "[${DATASET_NAME}]" > "${CONF}"
echo "titlePrefix= ${DATASET_NAME}" >> "${CONF}"
echo "RootSIFT= true" >> "${CONF}"
echo "SIFTscale3= true" >> "${CONF}"
echo "hammEmbBits= 64" >> "${CONF}"
echo "imagelistFn= ${IMLIST}" >> "${CONF}"
echo "databasePath= ${IMAGE_DIR}/" >> "${CONF}"
echo "dsetFn= ${VISE_DATADIR}dset.v2bin" >> "${CONF}"
echo "clstFn= ${VISE_DATADIR}clst.e3bin" >> "${CONF}"
echo "iidxFn= ${VISE_DATADIR}iidx.v2bin" >> "${CONF}"
echo "fidxFn= ${VISE_DATADIR}fidx.v2bin" >> "${CONF}"
echo "wghtFn= ${VISE_DATADIR}wght.v2bin" >> "${CONF}"
echo "tmpDir= ${TMPDIR}" >> "${CONF}"
echo "trainImagelistFn= ${IMLIST}" >> "${CONF}"
echo "trainDatabasePath= ${IMAGE_DIR}/" >> "${CONF}"
echo "trainNumDescs= 800000" >> "${CONF}"
echo "vocSize= 10000" >> "${CONF}"
echo "trainFilesPrefix= ${VISE_DATADIR}train_" >> "${CONF}"
echo "pathManHide= ${IMAGE_DIR}/" >> "${CONF}"

#
# train desc
#
printf "\ntrainDescs : \n"
TRAIN_DESC="${VISE_DATADIR}train_descs.e3bin"

if [[ ! -f $TRAIN_DESC ]]; then
  (cd "${VISE_ROOTDIR}/build";
  date ;
  ./v2/indexing/compute_index_v2 trainDescs "${DATASET_NAME}" "${CONF}" ;
  date )
else
  echo "*** Skipped trainDescs"
fi

#
# Computer clusters
#
printf "\ncompute_clusters : \n"
CLST="${VISE_DATADIR}clst.e3bin"

if [[ ! -f $CLST ]]; then
  (cd "${VISE_ROOTDIR}/build" ;
  date ;
  python ../src/v2/indexing/compute_clusters.py "${DATASET_NAME}" "${CONF}" ;
  date
  )
else
  echo "*** Skipped compute_clusters"
fi

#
# train assign
#
printf "\ntrainAssign : \n"
TRAIN_ASSIGN="${VISE_DATADIR}train_*_ assigns.bin"

if [[ ! -f $TRAIN_ASSIGN ]]; then
  (cd "${VISE_ROOTDIR}/build";
  date ;
  ./v2/indexing/compute_index_v2 trainAssign "${DATASET_NAME}" "${CONF}" ;
  date )
else
  echo "*** Skipped trainAssign"
fi


#
# train hamm
#
printf "\ntrainHamm : \n"
TRAIN_HAMM="${VISE_DATADIR}train_*_hamm64.v2bin"

if [[ ! -f $TRAIN_HAMM ]]; then
  (cd "${VISE_ROOTDIR}/build";
  date ;
  ./v2/indexing/compute_index_v2 trainHamm "${DATASET_NAME}" "${CONF}" ;
  date )
else
  echo "*** Skipped trainHamm"
fi

#
# perform indexing
#
printf "\nindex : \n"
TRAIN_INDEX="${VISE_DATADIR}dset.v2bin"

if [[ ! -f $TRAIN_INDEX ]]; then
  (cd "${VISE_ROOTDIR}/build";
  date ;
  ./v2/indexing/compute_index_v2 index "${DATASET_NAME}" "${CONF}" ;
  date )
else
  echo "*** Skipped index"
fi
printf "\nDone\n"
