#!/usr/bin/env bash

# Make bash exit if a command fails
set -e

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 [prod|dev] <docker compose args>"
    echo "Start anemolab service using existing docker images."
    echo "Example:"
    echo "   $0 dev up"
    exit 1
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

ENVIRONMENT=$1
shift

if [ ${ENVIRONMENT} == "prod" ]; then
  echo "Starting in prod mode. Changes in the www2 folder will be ignored."
  echo "http://localhost:9000/ will come up soon."
  docker-compose -f "${DIR}/docker-compose.yml" $*

elif [ ${ENVIRONMENT=} == "dev" ]; then
  echo "Starting in dev mode. Will live-reload any changes in the www2 folder."
  echo "http://localhost:9001/ will come up soon."
  FILE="docker-compose-dev.yml"
  PORT=9001
  mkdir -p /tmp/home
  USER_ID=$(id -u):$(id -g) docker-compose -f "${DIR}/${FILE}" ${*:-up}

else
  
  echo "Invalid option for run.sh please provide either 'prod' or 'dev'"
  exit 1
fi

