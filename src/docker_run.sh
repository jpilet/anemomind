#!/bin/bash
set -e

# Determine where is the root of the project based on where this script is
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." >/dev/null 2>&1 && pwd )"

# Everything starting with a '-' before the executable itself is considered as an argument for docker
DOCKER_ARGS=""
while [[ "$1" == "-"* ]] ; do
  DOCKER_ARGS="${DOCKER_ARGS} $1"
  shift
done

if [[ "$1" == "" ]] ; then 
  echo "Usage: $0 <docker options> <path to executable>"
  echo "note that the executable path is relative to the build directory within the docker image."
  exit 1
fi

# Make sure the docker image is up to date
docker build --target=build_env -t anemomind_build_env "${ROOT}"

# Create the build folder if needed
mkdir -p "${ROOT}/build_docker"

# Setting the home dir is required because some make install scripts create a ~/.cmake directory
docker run $DOCKER_ARGS -v "${ROOT}:/anemomind" -w /anemomind/build_docker \
         -u $(id -u):$(id -g)  -e HOME=/anemomind/build_docker anemomind_build_env $*

