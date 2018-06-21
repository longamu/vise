#!/bin/sh
HOSTNAME="localhost"
PORT="9973"
SEARCH_ENGINE_NAME="ox5k"
SEARCH_ENGINE_VERSION="1"

IMAGE_DATA_DIR="/home/tlm/dev/vise/data/test/ox5k"

for file in ${IMAGE_DATA_DIR}/*.jpg; do
  echo "Sending file ${file}"
  curl -X POST --data-binary "@${file}" "${HOSTNAME}:${PORT}/vise/repo/${SEARCH_ENGINE_NAME}/${SEARCH_ENGINE_VERSION}/add_image?filename=\"${file}\""
done

