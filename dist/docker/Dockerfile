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
ENV VISE_ROOT_DIR="/opt/vise/vise/"
ENV VISE_DEP_DIR="/opt/vise/build_dep/"

CMD ["uname", "-a"]


