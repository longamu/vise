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
ENV VGG_ROOT_DIR  = "/opt/vgg/"

#
# Prepare environment variables
#
RUN export VISE_ROOT_DIR=$VGG_ROOT_DIR"vise/"
RUN export VISE_DATA_DIR=$VISE_ROOT_DIR"vise_data/"
RUN export VISE_DEP_DIR=$VISE_ROOT_DIR"build_deps/"
RUN export VISE_DEP_LIBDIR=$VISE_ROOT_DIR"build_deps/lib/"
RUN export VISE_DEP_SRCDIR=$VISE_ROOT_DIR"build_deps/tmp_libsrc/"
RUN mkdir $VGG_ROOT_DIR $VISE_ROOT_DIR $VISE_DEP_DIR $VISE_DATA_DIR

#
# Install VISE dependencies
# 
RUN apt-get update
RUN apt-get install -y curl libpng12-dev libjpeg-dev libopenmpi-dev cmake python python-dev python-pip unzip
RUN pip install cherrypy numpy pillow cython
RUN apt-get install -y libmagick++-dev libprotobuf-dev protobuf-compiler libboost-atomic-dev libboost-chrono-dev libboost-filesystem-dev libboost-system-dev libboost-thread-dev libboost-timer-dev

#
# Install fastann
#
RUN export FASTANN_LIBDIR=$VISE_DEP_LIBDIR"fastann/"
RUN mkdir $VISE_DEP_SRCDIR
RUN cd $VISE_DEP_SRCDIR
RUN curl -o fastann.zip https://codeload.github.com/philbinj/fastann/zip/master
RUN unzip fastann.zip
RUN mv fastann-master fastann
RUN cd fastann
RUN mkdir build
RUN cd build
RUN export PREFIX=$FASTANN_LIBDIR
RUN cmake ../
RUN make -j8
RUN make install


#
# Download VISE source : http://vgg.gitlab.io/vise/release/vise-1.0.0-beta.tar.gz
#
RUN cd $VISE_ROOT_DIR
RUN curl -o vise-1.0.0-beta.tar.gz http://vgg.gitlab.io/vise/release/vise-1.0.0-beta.tar.gz
RUN tar -zxf vise-1.0.0-beta.tar.gz
RUN cd vise-1.0.0-beta
RUN export VISE_SRC_DIR=`pwd`
RUN mkdir build
RUN cd build
RUN cmake -DCMAKE_PREFIX_PATH=$FASTANN_LIBDIR ../src
RUN make -j8
RUN ./vise $VISE_SRC_DIR $VISE_DATA_DIR
