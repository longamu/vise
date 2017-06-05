#
# Dockerfile for creating a docker image that runs the VGG Image Search Engine 
# (VISE).
#
# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# Date: 05 June 2017
#

#
# Install Debian as base image (tightly controlled and minimal, 150 MB)
# See: https://hub.docker.com/_/debian/
#
FROM debian:jessie
RUN ["echo", "Downloading Debian as base image"]

#
# Define image labels
#
LABEL name        = "VGG Image Search Engine"
LABEL codename    = "VISE"
LABEL version     = "1.0.0"
LABEL maintainer  = "Abhishek Dutta <adutta@robots.ox.ac.uk>"
LABEL description = "VGG Image Search Engine (VISE) enables image instance based search of a large collection of images."

#
# Define ports used by VISE
#   VISE trainer/loader : 8080
#   VISE loaded engine backend  : 9971
#   VISE loaded engine frontend : 9972
#
EXPOSE 8080 9971 9972
#
# Define environment variables
#
ENV VGG_ROOT_DIR    "/opt/vgg/"
ENV VISE_ROOT_DIR   "/opt/vgg/vise/"
ENV VISE_CODE_DIR   "/opt/vgg/vise/vise-1.0.0-beta/"
ENV VISE_DATA_DIR   "/opt/vgg/vise/vise_data/"
ENV VISE_DEP_DIR    "/opt/vgg/vise/build_deps/"
ENV VISE_DEP_LIBDIR "/opt/vgg/vise/build_deps/lib/"
ENV VISE_DEP_SRCDIR "/opt/vgg/vise/build_deps/tmp_libsrc/"

#
# Prepare environment variables
#
RUN mkdir $VGG_ROOT_DIR $VISE_ROOT_DIR $VISE_CODE_DIR $VISE_DATA_DIR $VISE_DEP_DIR $VISE_DEP_SRCDIR $VISE_DEP_LIBDIR

#
# Install VISE dependencies
# 
RUN apt-get update
RUN apt-get install -y curl libpng12-dev libjpeg-dev libopenmpi-dev cmake python python-dev python-pip unzip libmagick++-dev libprotobuf-dev protobuf-compiler libboost-atomic-dev libboost-chrono-dev libboost-filesystem-dev libboost-system-dev libboost-thread-dev libboost-timer-dev
RUN pip install cherrypy numpy pillow cython


#
# Install fastann
#
RUN export FASTANN_LIBDIR=$VISE_DEP_LIBDIR"fastann/" &&
 cd $VISE_DEP_SRCDIR &&
 curl -o fastann.zip https://codeload.github.com/philbinj/fastann/zip/master &&
 unzip fastann.zip &&
 mv fastann-master fastann &&
 cd fastann &&
 mkdir build &&
 cd build &&
 export PREFIX=$FASTANN_LIBDIR && 
 cmake ../ && make -j8 && make install


#
# Download VISE source : http://vgg.gitlab.io/vise/release/vise-1.0.0-beta.tar.gz
#
RUN cd $VISE_CODE_DIR &&
 curl -o vise-1.0.0-beta.tar.gz http://vgg.gitlab.io/vise/release/vise-1.0.0-beta.tar.gz &&
 tar -zxf vise-1.0.0-beta.tar.gz &&
 mkdir build &&
 cd build &&
 cmake -DCMAKE_PREFIX_PATH=$FASTANN_LIBDIR ../src &&
 make -j8 &&
 ./vise $VISE_CODE_DIR $VISE_DATA_DIR


